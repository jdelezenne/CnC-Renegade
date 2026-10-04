#include "D3D8.h"
#include "Platform/Platform.h"
#include <SDL3/SDL.h>
#include <d3d8.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <span>
#include <vector>
#include <utility>
#include <type_traits>
#include "FixedVertexDxil.h"
#include "FixedVertexSpirv.h"
#include "FixedFragmentDxil.h"
#include "FixedFragmentSpirv.h"
#include "PresentVertexDxil.h"
#include "PresentVertexSpirv.h"
#include "PresentFragmentDxil.h"
#include "PresentFragmentSpirv.h"

namespace {
class Device;
class Factory;
struct Image;

HRESULT Failure(const char* operation)
{
    SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s: %s", operation, SDL_GetError());
    return D3DERR_NOTAVAILABLE;
}
HRESULT Unsupported(const char* operation)
{
    SDL_LogError(SDL_LOG_CATEGORY_GPU, "Unsupported D3D8 call: %s", operation);
    return D3DERR_NOTAVAILABLE;
}
struct Float4 { float x = 0, y = 0, z = 0, w = 0; };
struct Int4 { int x = 0, y = 0, z = 0, w = 0; };
using Matrix = std::array<float, 16>;
Matrix Identity()
{
    Matrix value{};
    value[0] = value[5] = value[10] = value[15] = 1;
    return value;
}
Float4 Color(DWORD color)
{
    return { ((color >> 16) & 255) / 255.0f, ((color >> 8) & 255) / 255.0f,
        (color & 255) / 255.0f, (color >> 24) / 255.0f };
}
Float4 Vector(const D3DVECTOR& value, float w = 0) { return { value.x, value.y, value.z, w }; }
Float4 Color(const D3DCOLORVALUE& value) { return { value.r, value.g, value.b, value.a }; }
Float4 Transform(Float4 p, const Matrix& m)
{
    return { p.x*m[0]+p.y*m[4]+p.z*m[8]+p.w*m[12], p.x*m[1]+p.y*m[5]+p.z*m[9]+p.w*m[13],
        p.x*m[2]+p.y*m[6]+p.z*m[10]+p.w*m[14], p.x*m[3]+p.y*m[7]+p.z*m[11]+p.w*m[15] };
}
Matrix Multiply(const Matrix& a, const Matrix& b)
{
    Matrix result{};
    for (int row = 0; row < 4; ++row) for (int col = 0; col < 4; ++col)
        for (int k = 0; k < 4; ++k) result[row*4+col] += a[row*4+k] * b[k*4+col];
    return result;
}
Matrix NormalMatrix(const Matrix& m)
{
    float a=m[0], b=m[1], c=m[2], d=m[4], e=m[5], f=m[6], g=m[8], h=m[9], i=m[10];
    float determinant=a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g);
    if (std::abs(determinant) < 1e-20f) return Identity();
    Matrix result{};
    result[0]=(e*i-f*h)/determinant; result[1]=(f*g-d*i)/determinant; result[2]=(d*h-e*g)/determinant;
    result[4]=(c*h-b*i)/determinant; result[5]=(a*i-c*g)/determinant; result[6]=(b*g-a*h)/determinant;
    result[8]=(b*f-c*e)/determinant; result[9]=(c*d-a*f)/determinant; result[10]=(a*e-b*d)/determinant;
    result[15]=1;
    return result;
}
struct Vertex {
    Float4 Position{0,0,0,1};
    Float4 Normal{0,0,1,0};
    Float4 Diffuse{1,1,1,1};
    Float4 Specular{};
    std::array<Float4,8> Coordinates{};
};
struct LightState {
    Float4 Position, Direction, Diffuse, Ambient, Specular, Parameters, Settings;
};
struct VertexState {
    Matrix World, View, Projection, Normal;
    std::array<Matrix,8> TextureTransforms;
    Float4 Viewport;
    Int4 Flags;
    Float4 Diffuse, Ambient, Specular, Emissive, GlobalAmbient, Settings;
    Int4 Sources;
    std::array<Int4,8> Coordinates;
    std::array<LightState,8> Lights;
    Float4 Fog;
    Int4 FogSettings;
};
struct StageState { Int4 Color, Alpha, State; Float4 Sampling, Bump, Border; };
struct FragmentState {
    Float4 Factor, FogColor, Fog;
    Int4 Flags;
    Float4 Alpha;
    std::array<StageState,8> Stages;
};

unsigned BytesPerPixel(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_A8: case D3DFMT_L8: case D3DFMT_P8: case D3DFMT_A4L4: return 1;
    case D3DFMT_R5G6B5: case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5: case D3DFMT_A4R4G4B4:
    case D3DFMT_A8L8: case D3DFMT_V8U8: case D3DFMT_L6V5U5: case D3DFMT_D16: case D3DFMT_D15S1: return 2;
    case D3DFMT_R8G8B8: return 3;
    case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: case D3DFMT_X8L8V8U8: case D3DFMT_Q8W8V8U8:
    case D3DFMT_V16U16: case D3DFMT_D24S8: case D3DFMT_D24X8: case D3DFMT_D24X4S4: case D3DFMT_D32: return 4;
    default: return 0;
    }
}
unsigned BlockBytes(D3DFORMAT format)
{
    if (format == D3DFMT_DXT1) return 8;
    if (format >= D3DFMT_DXT2 && format <= D3DFMT_DXT5) return 16;
    return 0;
}
bool DepthFormat(D3DFORMAT format)
{
    return format == D3DFMT_D16 || format == D3DFMT_D15S1 || format == D3DFMT_D24S8 ||
        format == D3DFMT_D24X8 || format == D3DFMT_D24X4S4 || format == D3DFMT_D32;
}
SDL_GPUTextureFormat GPUFormat(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_DXT1: return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
    case D3DFMT_DXT2: case D3DFMT_DXT3: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case D3DFMT_DXT4: case D3DFMT_DXT5: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    case D3DFMT_D16: return SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    case D3DFMT_D15S1: case D3DFMT_D24S8: case D3DFMT_D24X4S4: return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    case D3DFMT_D24X8: case D3DFMT_D32: return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    case D3DFMT_V8U8: case D3DFMT_L6V5U5: case D3DFMT_X8L8V8U8:
    case D3DFMT_Q8W8V8U8: case D3DFMT_V16U16: return SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
    default: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    }
}
bool HasStencil(SDL_GPUTextureFormat format)
{
    return format == SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT || format == SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
}
struct Context {
    SDL_GPUDevice* GPU = nullptr;
    SDL_GPUCommandBuffer* Commands = nullptr;
    SDL_GPURenderPass* Pass = nullptr;
    SDL_Window* Window = nullptr;
    Device* Owner = nullptr;
    SDL_GPUShader *VS=nullptr, *PS=nullptr, *PresentVS=nullptr, *PresentPS=nullptr;
    bool Spirv = false;
    bool Claimed = false;
    std::array<PALETTEENTRY,256> Palette{};
    unsigned PaletteVersion = 1;
    std::vector<std::weak_ptr<Image>> ManagedImages;

    ~Context();
    bool Initialize();
    bool Command();
    void EndPass();
    bool Flush(bool wait);
    bool CreateImage(Image& image);
    bool Upload(Image& image);
    bool ReadImage(Image& image, unsigned level);
    SDL_GPUBuffer* UploadBuffer(const void* data, unsigned bytes, SDL_GPUBufferUsageFlags usage, SDL_GPUBuffer* buffer = nullptr);
};
struct ImageLevel {
    unsigned Width=0, Height=0, Pitch=0;
    std::vector<unsigned char> Bytes;
    bool Dirty=true, GPUWritten=false, Locked=false, ReadOnly=false;
};
struct Image {
    std::shared_ptr<Context> Ctx;
    SDL_GPUTexture* GPU = nullptr;
    D3DFORMAT Format;
    D3DPOOL Pool;
    DWORD Usage;
    SDL_GPUTextureFormat ActualFormat;
    std::vector<ImageLevel> Levels;
    unsigned PaletteVersion=0;
    Image(std::shared_ptr<Context> ctx, unsigned width, unsigned height, unsigned levels, D3DFORMAT format, D3DPOOL pool, DWORD usage)
        : Ctx(std::move(ctx)), Format(format), Pool(pool), Usage(usage), ActualFormat(GPUFormat(format))
    {
        if (!levels) { levels=1; for (unsigned size=std::max(width,height); size>1; size>>=1) ++levels; }
        unsigned block=BlockBytes(format), pixel=BytesPerPixel(format);
        for (unsigned i=0;i<levels;++i) {
            ImageLevel level;
            level.Width=std::max(width>>i,1u); level.Height=std::max(height>>i,1u);
            level.Pitch=block ? ((level.Width+3)/4)*block : level.Width*pixel;
            unsigned rows=block ? (level.Height+3)/4 : level.Height;
            level.Bytes.resize(static_cast<size_t>(level.Pitch)*rows);
            Levels.push_back(std::move(level));
        }
    }
    ~Image() { if (GPU) SDL_ReleaseGPUTexture(Ctx->GPU,GPU); }
    HRESULT Desc(unsigned index, D3DSURFACE_DESC* desc) const
    {
        if (!desc || index>=Levels.size()) return D3DERR_INVALIDCALL;
        auto& level=Levels[index];
        *desc={Format,D3DRTYPE_SURFACE,Usage,Pool,static_cast<UINT>(level.Bytes.size()),D3DMULTISAMPLE_NONE,level.Width,level.Height};
        return D3D_OK;
    }
    HRESULT Lock(unsigned index, D3DLOCKED_RECT* output, const RECT* rect, DWORD flags)
    {
        if (!output || index>=Levels.size() || DepthFormat(Format)) return D3DERR_INVALIDCALL;
        auto& level=Levels[index];
        if (level.Locked) return D3DERR_INVALIDCALL;
        if (level.GPUWritten && !Ctx->ReadImage(*this,index)) return Failure("Read render target");
        unsigned left=rect ? rect->left : 0, top=rect ? rect->top : 0;
        if (rect && (rect->left<0 || rect->top<0 || rect->right>static_cast<LONG>(level.Width) ||
            rect->bottom>static_cast<LONG>(level.Height) || rect->right<=rect->left || rect->bottom<=rect->top)) return D3DERR_INVALIDCALL;
        unsigned block=BlockBytes(Format);
        if (block && ((left|top)&3)) return D3DERR_INVALIDCALL;
        size_t offset=block ? (top/4)*level.Pitch+(left/4)*block : top*level.Pitch+left*BytesPerPixel(Format);
        output->Pitch=level.Pitch; output->pBits=level.Bytes.data()+offset;
        level.Locked=true; level.ReadOnly=(flags&D3DLOCK_READONLY)!=0;
        return D3D_OK;
    }
    HRESULT Unlock(unsigned index)
    {
        if (index>=Levels.size() || !Levels[index].Locked) return D3DERR_INVALIDCALL;
        auto& level=Levels[index]; level.Locked=false;
        if (!level.ReadOnly) { level.Dirty=true; level.GPUWritten=false; }
        return D3D_OK;
    }
};
struct PrivateValue {
    std::vector<unsigned char> Bytes;
    IUnknown* Object=nullptr;
    ~PrivateValue() { if (Object) Object->Release(); }
};
struct PrivateData {
    std::map<std::array<unsigned char,16>,std::unique_ptr<PrivateValue>> Values;
    static std::array<unsigned char,16> Key(REFGUID guid) { std::array<unsigned char,16> key; std::memcpy(key.data(),&guid,16); return key; }
    HRESULT Set(REFGUID guid,const void* data,DWORD bytes,DWORD flags)
    {
        if (!data || (flags & ~D3DSPD_IUNKNOWN)) return D3DERR_INVALIDCALL;
        auto value=std::make_unique<PrivateValue>();
        if (flags&D3DSPD_IUNKNOWN) {
            if (bytes!=sizeof(IUnknown*)) return D3DERR_INVALIDCALL;
            value->Object=static_cast<IUnknown*>(const_cast<void*>(data)); value->Object->AddRef();
        } else value->Bytes.assign(static_cast<const unsigned char*>(data),static_cast<const unsigned char*>(data)+bytes);
        Values[Key(guid)]=std::move(value); return D3D_OK;
    }
    HRESULT Get(REFGUID guid,void* output,DWORD* bytes)
    {
        if (!bytes) return D3DERR_INVALIDCALL;
        auto found=Values.find(Key(guid)); if (found==Values.end()) return D3DERR_NOTFOUND;
        auto& value=*found->second;
        DWORD size=value.Object ? sizeof(IUnknown*) : static_cast<DWORD>(value.Bytes.size());
        if (*bytes<size || !output) { *bytes=size; return D3DERR_MOREDATA; }
        *bytes=size;
        if (value.Object) { std::memcpy(output,&value.Object,size); value.Object->AddRef(); }
        else std::memcpy(output,value.Bytes.data(),size);
        return D3D_OK;
    }
    HRESULT Free(REFGUID guid) { return Values.erase(Key(guid)) ? D3D_OK : D3DERR_NOTFOUND; }
};
template<class T> const GUID& InterfaceId();
template<> const GUID& InterfaceId<IDirect3D8>() { return IID_IDirect3D8; }
template<> const GUID& InterfaceId<IDirect3DDevice8>() { return IID_IDirect3DDevice8; }
template<> const GUID& InterfaceId<IDirect3DTexture8>() { return IID_IDirect3DTexture8; }
template<> const GUID& InterfaceId<IDirect3DSurface8>() { return IID_IDirect3DSurface8; }
template<> const GUID& InterfaceId<IDirect3DVertexBuffer8>() { return IID_IDirect3DVertexBuffer8; }
template<> const GUID& InterfaceId<IDirect3DIndexBuffer8>() { return IID_IDirect3DIndexBuffer8; }
template<> const GUID& InterfaceId<IDirect3DSwapChain8>() { return IID_IDirect3DSwapChain8; }
template<class T> struct Object : T {
    std::atomic<ULONG> References{1};
    PrivateData Private;
    virtual ~Object() = default;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** result) override
    {
        if (!result) return E_POINTER;
        *result=nullptr;
        bool supported=IsEqualGUID(iid,IID_IUnknown) || IsEqualGUID(iid,InterfaceId<T>());
        if constexpr(std::is_base_of_v<IDirect3DResource8,T>) supported=supported || IsEqualGUID(iid,IID_IDirect3DResource8)!=0;
        if constexpr(std::is_base_of_v<IDirect3DBaseTexture8,T>) supported=supported || IsEqualGUID(iid,IID_IDirect3DBaseTexture8)!=0;
        if (!supported) return E_NOINTERFACE;
        *result=static_cast<T*>(this); AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++References; }
    ULONG STDMETHODCALLTYPE Release() override { ULONG refs=--References; if (!refs) delete this; return refs; }
};
HRESULT GetOwner(Context& context, IDirect3DDevice8** output);
template<class T,D3DRESOURCETYPE Type> struct Resource : Object<T> {
    std::shared_ptr<Context> Ctx;
    DWORD Priority=0;
    explicit Resource(std::shared_ptr<Context> ctx):Ctx(std::move(ctx)) {}
    HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** result) override { return GetOwner(*Ctx,result); }
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID g,const void* d,DWORD n,DWORD f) override { return this->Private.Set(g,d,n,f); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID g,void* d,DWORD* n) override { return this->Private.Get(g,d,n); }
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID g) override { return this->Private.Free(g); }
    DWORD STDMETHODCALLTYPE SetPriority(DWORD priority) override { return std::exchange(Priority,priority); }
    DWORD STDMETHODCALLTYPE GetPriority() override { return Priority; }
    void STDMETHODCALLTYPE PreLoad() override = 0;
    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return Type; }
};
struct Surface final : Object<IDirect3DSurface8> {
    std::shared_ptr<Image> Data;
    unsigned Level;
    IUnknown* Parent;
    Surface(std::shared_ptr<Image> image,unsigned level=0,IUnknown* parent=nullptr):Data(std::move(image)),Level(level),Parent(parent) { if (Parent) Parent->AddRef(); }
    ~Surface() override { if (Parent) Parent->Release(); }
    HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** result) override { return GetOwner(*Data->Ctx,result); }
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID g,const void* d,DWORD n,DWORD f) override { return Private.Set(g,d,n,f); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID g,void* d,DWORD* n) override { return Private.Get(g,d,n); }
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID g) override { return Private.Free(g); }
    HRESULT STDMETHODCALLTYPE GetContainer(REFIID iid,void** output) override
    {
        if (Parent) return Parent->QueryInterface(iid,output);
        IDirect3DDevice8* device=nullptr; HRESULT result=GetOwner(*Data->Ctx,&device);
        if (FAILED(result)) return result;
        result=device->QueryInterface(iid,output); device->Release(); return result;
    }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC* desc) override { return Data->Desc(Level,desc); }
    HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* out,const RECT* rect,DWORD flags) override { return Data->Lock(Level,out,rect,flags); }
    HRESULT STDMETHODCALLTYPE UnlockRect() override { return Data->Unlock(Level); }
};
struct Texture final : Resource<IDirect3DTexture8,D3DRTYPE_TEXTURE> {
    std::shared_ptr<Image> Data;
    DWORD LOD=0;
    explicit Texture(std::shared_ptr<Image> image):Resource(image->Ctx),Data(std::move(image)) {}
    void STDMETHODCALLTYPE PreLoad() override { if(!Ctx->Upload(*Data))Failure("Preload texture"); }
    DWORD STDMETHODCALLTYPE SetLOD(DWORD lod) override { return std::exchange(LOD,std::min<DWORD>(lod,Data->Levels.size()-1)); }
    DWORD STDMETHODCALLTYPE GetLOD() override { return LOD; }
    DWORD STDMETHODCALLTYPE GetLevelCount() override { return Data->Levels.size(); }
    HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT level,D3DSURFACE_DESC* desc) override { return Data->Desc(level,desc); }
    HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT level,IDirect3DSurface8** result) override
    {
        if (!result || level>=Data->Levels.size()) return D3DERR_INVALIDCALL;
        *result=new Surface(Data,level,this); return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE LockRect(UINT level,D3DLOCKED_RECT* out,const RECT* rect,DWORD flags) override { return Data->Lock(level,out,rect,flags); }
    HRESULT STDMETHODCALLTYPE UnlockRect(UINT level) override { return Data->Unlock(level); }
    HRESULT STDMETHODCALLTYPE AddDirtyRect(const RECT*) override { for (auto& level:Data->Levels) level.Dirty=true; return D3D_OK; }
};
struct BufferData {
    std::shared_ptr<Context> Ctx;
    std::vector<unsigned char> Bytes;
    SDL_GPUBuffer* GPU=nullptr;
    DWORD Usage=0,FVF=0;
    D3DFORMAT Format=D3DFMT_UNKNOWN;
    D3DPOOL Pool=D3DPOOL_DEFAULT;
    unsigned GPUSize=0,GPUStride=0;
    DWORD GPUFVF=0;
    bool Dirty=true,Locked=false,ReadOnly=false;
    BufferData(std::shared_ptr<Context> ctx,unsigned bytes,DWORD usage,D3DPOOL pool):Ctx(std::move(ctx)),Bytes(bytes),Usage(usage),Pool(pool) {}
    ~BufferData() { if (GPU) SDL_ReleaseGPUBuffer(Ctx->GPU,GPU); }
    HRESULT Lock(unsigned offset,unsigned length,BYTE** data,DWORD flags)
    {
        if (!data || Locked || offset>=Bytes.size() || (length && length>Bytes.size()-offset)) return D3DERR_INVALIDCALL;
        *data=Bytes.data()+offset; Locked=true; ReadOnly=(flags&D3DLOCK_READONLY)!=0; return D3D_OK;
    }
    HRESULT Unlock() { if (!Locked) return D3DERR_INVALIDCALL; Locked=false; if (!ReadOnly) Dirty=true; return D3D_OK; }
};
struct VertexBuffer final : Resource<IDirect3DVertexBuffer8,D3DRTYPE_VERTEXBUFFER> {
    BufferData Data;
    VertexBuffer(std::shared_ptr<Context> ctx,unsigned bytes,DWORD usage,DWORD fvf,D3DPOOL pool):Resource(ctx),Data(ctx,bytes,usage,pool) { Data.FVF=fvf; }
    void STDMETHODCALLTYPE PreLoad() override;
    HRESULT STDMETHODCALLTYPE Lock(UINT offset,UINT length,BYTE** data,DWORD flags) override { return Data.Lock(offset,length,data,flags); }
    HRESULT STDMETHODCALLTYPE Unlock() override { return Data.Unlock(); }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC* desc) override
    { if (!desc) return D3DERR_INVALIDCALL; *desc={D3DFMT_VERTEXDATA,D3DRTYPE_VERTEXBUFFER,Data.Usage,Data.Pool,static_cast<UINT>(Data.Bytes.size()),Data.FVF}; return D3D_OK; }
};
struct IndexBuffer final : Resource<IDirect3DIndexBuffer8,D3DRTYPE_INDEXBUFFER> {
    BufferData Data;
    IndexBuffer(std::shared_ptr<Context> ctx,unsigned bytes,DWORD usage,D3DFORMAT format,D3DPOOL pool):Resource(ctx),Data(ctx,bytes,usage,pool) { Data.Format=format; }
    void STDMETHODCALLTYPE PreLoad() override;
    HRESULT STDMETHODCALLTYPE Lock(UINT offset,UINT length,BYTE** data,DWORD flags) override { return Data.Lock(offset,length,data,flags); }
    HRESULT STDMETHODCALLTYPE Unlock() override { return Data.Unlock(); }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC* desc) override
    { if (!desc) return D3DERR_INVALIDCALL; *desc={Data.Format,D3DRTYPE_INDEXBUFFER,Data.Usage,Data.Pool,static_cast<UINT>(Data.Bytes.size())}; return D3D_OK; }
};

std::uint16_t Half(float f)
{
    unsigned bits=std::bit_cast<unsigned>(f), sign=(bits>>16)&0x8000;
    int exponent=static_cast<int>((bits>>23)&255)-112;
    unsigned mantissa=bits&0x7fffff;
    if (exponent<=0) {
        if (exponent<-10) return static_cast<std::uint16_t>(sign);
        mantissa=(mantissa|0x800000)>>(1-exponent);
        return static_cast<std::uint16_t>(sign|((mantissa+0x1000)>>13));
    }
    if (exponent>=31) return static_cast<std::uint16_t>(sign|0x7c00);
    return static_cast<std::uint16_t>(sign|(exponent<<10)|((mantissa+0x1000)>>13));
}
std::vector<unsigned char> Pixels(const Image& image,unsigned index)
{
    auto& level=image.Levels[index];
    if (BlockBytes(image.Format)) return level.Bytes;
    bool bump=image.ActualFormat==SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
    std::vector<unsigned char> result(static_cast<size_t>(level.Width)*level.Height*(bump?8:4));
    unsigned bytes=BytesPerPixel(image.Format);
    for (size_t pixel=0;pixel<static_cast<size_t>(level.Width)*level.Height;++pixel) {
        auto* p=level.Bytes.data()+pixel*bytes;
        unsigned value=0; std::memcpy(&value,p,bytes);
        Float4 color{0,0,0,1};
        switch (image.Format) {
        case D3DFMT_A8R8G8B8: color=Color(value); break;
        case D3DFMT_X8R8G8B8: case D3DFMT_R8G8B8: color=Color(value|0xff000000); break;
        case D3DFMT_R5G6B5: color={((value>>11)&31)/31.0f,((value>>5)&63)/63.0f,(value&31)/31.0f,1}; break;
        case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5: color={((value>>10)&31)/31.0f,((value>>5)&31)/31.0f,(value&31)/31.0f,image.Format==D3DFMT_X1R5G5B5?1.0f:static_cast<float>(value>>15)}; break;
        case D3DFMT_A4R4G4B4: color={((value>>8)&15)/15.0f,((value>>4)&15)/15.0f,(value&15)/15.0f,(value>>12)/15.0f}; break;
        case D3DFMT_L8: color={p[0]/255.0f,p[0]/255.0f,p[0]/255.0f,1}; break;
        case D3DFMT_A8: color={1,1,1,p[0]/255.0f}; break;
        case D3DFMT_A8L8: color={p[0]/255.0f,p[0]/255.0f,p[0]/255.0f,p[1]/255.0f}; break;
        case D3DFMT_A4L4: color={(value&15)/15.0f,(value&15)/15.0f,(value&15)/15.0f,(value>>4)/15.0f}; break;
        case D3DFMT_P8: { auto entry=image.Ctx->Palette[p[0]]; color={entry.peRed/255.0f,entry.peGreen/255.0f,entry.peBlue/255.0f,entry.peFlags/255.0f}; break; }
        case D3DFMT_V8U8: color={std::max(-1.0f,static_cast<signed char>(p[0])/127.0f),std::max(-1.0f,static_cast<signed char>(p[1])/127.0f),0,1}; break;
        case D3DFMT_L6V5U5: { int u=(value&31),v=(value>>5)&31; if(u&16)u-=32;if(v&16)v-=32; color={std::max(-1.0f,u/15.0f),std::max(-1.0f,v/15.0f),(value>>10)/63.0f,1}; break; }
        case D3DFMT_X8L8V8U8: color={std::max(-1.0f,static_cast<signed char>(p[0])/127.0f),std::max(-1.0f,static_cast<signed char>(p[1])/127.0f),p[2]/255.0f,1}; break;
        case D3DFMT_V16U16: color={std::max(-1.0f,static_cast<short>(value&65535)/32767.0f),std::max(-1.0f,static_cast<short>(value>>16)/32767.0f),0,1}; break;
        case D3DFMT_Q8W8V8U8: color={std::max(-1.0f,static_cast<signed char>(p[0])/127.0f),std::max(-1.0f,static_cast<signed char>(p[1])/127.0f),std::max(-1.0f,static_cast<signed char>(p[2])/127.0f),std::max(-1.0f,static_cast<signed char>(p[3])/127.0f)}; break;
        default: break;
        }
        if (bump) { std::uint16_t half[4]{Half(color.x),Half(color.y),Half(color.z),Half(color.w)}; std::memcpy(result.data()+pixel*8,half,8); }
        else {
            float channels[]{color.x,color.y,color.z,color.w};
            for(unsigned i=0;i<4;++i) result[pixel*4+i]=static_cast<unsigned char>(std::clamp(channels[i]*255+0.5f,0.0f,255.0f));
        }
    }
    return result;
}
bool Context::Command() { if (!Commands) Commands=SDL_AcquireGPUCommandBuffer(GPU); return Commands!=nullptr; }
void Context::EndPass() { if (Pass) SDL_EndGPURenderPass(Pass); Pass=nullptr; }
bool Context::Flush(bool wait)
{
    EndPass(); if (!Commands) return true;
    auto* commands=std::exchange(Commands,nullptr);
    if (!wait) return SDL_SubmitGPUCommandBuffer(commands);
    auto* fence=SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    if (!fence) return false;
    bool result=SDL_WaitForGPUFences(GPU,true,&fence,1); SDL_ReleaseGPUFence(GPU,fence); return result;
}
bool Context::CreateImage(Image& image)
{
    if (image.GPU) return true;
    auto& level=image.Levels.front();
    SDL_GPUTextureCreateInfo info{};
    info.type=SDL_GPU_TEXTURETYPE_2D; info.format=image.ActualFormat;
    info.usage=DepthFormat(image.Format)?SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET:SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if (image.Usage&D3DUSAGE_RENDERTARGET) info.usage|=SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    if (!SDL_GPUTextureSupportsFormat(GPU,info.format,info.type,info.usage) && info.format==SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT)
        image.ActualFormat=info.format=SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
    info.width=level.Width; info.height=level.Height; info.layer_count_or_depth=1;
    info.num_levels=image.Levels.size(); info.sample_count=SDL_GPU_SAMPLECOUNT_1;
    image.GPU=SDL_CreateGPUTexture(GPU,&info);
    if (image.Usage&(D3DUSAGE_RENDERTARGET|D3DUSAGE_DEPTHSTENCIL)) for(auto& mip:image.Levels)mip.Dirty=false;
    return image.GPU!=nullptr;
}
bool Context::Upload(Image& image)
{
    if (!CreateImage(image)) return false;
    bool dirty=image.Format==D3DFMT_P8 && image.PaletteVersion!=PaletteVersion;
    for (auto& level:image.Levels) dirty|=level.Dirty;
    if (!dirty || DepthFormat(image.Format)) return true;
    for (unsigned i=0;i<image.Levels.size();++i) if(image.Levels[i].GPUWritten && !ReadImage(image,i))return false;
    EndPass(); if (!Command()) return false;
    auto* pass=SDL_BeginGPUCopyPass(Commands);
    bool result=true;
    for (unsigned i=0;i<image.Levels.size();++i) {
        auto& level=image.Levels[i]; auto pixels=Pixels(image,i);
        SDL_GPUTransferBufferCreateInfo info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,static_cast<Uint32>(pixels.size()),0};
        auto* transfer=SDL_CreateGPUTransferBuffer(GPU,&info);
        if (!transfer) { result=false; break; }
        auto* mapping=SDL_MapGPUTransferBuffer(GPU,transfer,false);
        if (!mapping) { SDL_ReleaseGPUTransferBuffer(GPU,transfer); result=false; break; }
        std::memcpy(mapping,pixels.data(),pixels.size()); SDL_UnmapGPUTransferBuffer(GPU,transfer);
        SDL_GPUTextureTransferInfo source{transfer,0,BlockBytes(image.Format)?((level.Width+3)&~3u):level.Width,BlockBytes(image.Format)?((level.Height+3)&~3u):level.Height};
        SDL_GPUTextureRegion destination{image.GPU,i,0,0,0,0,level.Width,level.Height,1};
        SDL_UploadToGPUTexture(pass,&source,&destination,i==0);
        SDL_ReleaseGPUTransferBuffer(GPU,transfer); level.Dirty=false;
    }
    SDL_EndGPUCopyPass(pass); image.PaletteVersion=PaletteVersion; return result;
}
SDL_GPUBuffer* Context::UploadBuffer(const void* data,unsigned bytes,SDL_GPUBufferUsageFlags usage,SDL_GPUBuffer* buffer)
{
    EndPass(); if (!Command() || !bytes) return nullptr;
    bool created=!buffer;
    if (!buffer) { SDL_GPUBufferCreateInfo info{usage,bytes,0}; buffer=SDL_CreateGPUBuffer(GPU,&info); }
    if (!buffer) return nullptr;
    SDL_GPUTransferBufferCreateInfo info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,bytes,0};
    auto* transfer=SDL_CreateGPUTransferBuffer(GPU,&info);
    auto* mapping=transfer ? SDL_MapGPUTransferBuffer(GPU,transfer,false) : nullptr;
    if (!mapping) { if(transfer)SDL_ReleaseGPUTransferBuffer(GPU,transfer);if(created)SDL_ReleaseGPUBuffer(GPU,buffer);return nullptr; }
    std::memcpy(mapping,data,bytes); SDL_UnmapGPUTransferBuffer(GPU,transfer);
    auto* pass=SDL_BeginGPUCopyPass(Commands);
    SDL_GPUTransferBufferLocation source{transfer,0}; SDL_GPUBufferRegion dest{buffer,0,bytes};
    SDL_UploadToGPUBuffer(pass,&source,&dest,true); SDL_EndGPUCopyPass(pass);
    SDL_ReleaseGPUTransferBuffer(GPU,transfer); return buffer;
}
bool Context::ReadImage(Image& image,unsigned index)
{
    if (!image.GPU || index>=image.Levels.size() || image.ActualFormat!=SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM) return false;
    EndPass(); if (!Command()) return false;
    auto& level=image.Levels[index];
    SDL_GPUTransferBufferCreateInfo info{SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,level.Width*level.Height*4,0};
    auto* transfer=SDL_CreateGPUTransferBuffer(GPU,&info); if (!transfer) return false;
    auto* pass=SDL_BeginGPUCopyPass(Commands);
    SDL_GPUTextureRegion source{image.GPU,index,0,0,0,0,level.Width,level.Height,1};
    SDL_GPUTextureTransferInfo dest{transfer,0,level.Width,level.Height};
    SDL_DownloadFromGPUTexture(pass,&source,&dest); SDL_EndGPUCopyPass(pass);
    bool result=Flush(true);
    auto* pixels=result ? static_cast<unsigned char*>(SDL_MapGPUTransferBuffer(GPU,transfer,false)) : nullptr;
    if (pixels) {
        for (size_t i=0;i<static_cast<size_t>(level.Width)*level.Height;++i) {
            auto* p=pixels+i*4; DWORD color=(static_cast<DWORD>(p[3])<<24)|(p[0]<<16)|(p[1]<<8)|p[2];
            auto* output=level.Bytes.data()+i*BytesPerPixel(image.Format);
            unsigned value=color;
            if(image.Format==D3DFMT_R5G6B5)value=((p[0]>>3)<<11)|((p[1]>>2)<<5)|(p[2]>>3);
            if(image.Format==D3DFMT_A1R5G5B5||image.Format==D3DFMT_X1R5G5B5)value=((p[3]>>7)<<15)|((p[0]>>3)<<10)|((p[1]>>3)<<5)|(p[2]>>3);
            if(image.Format==D3DFMT_A4R4G4B4)value=((p[3]>>4)<<12)|((p[0]>>4)<<8)|((p[1]>>4)<<4)|(p[2]>>4);
            std::memcpy(output,&value,BytesPerPixel(image.Format));
        }
        SDL_UnmapGPUTransferBuffer(GPU,transfer); level.GPUWritten=false;
    } else result=false;
    SDL_ReleaseGPUTransferBuffer(GPU,transfer); return result;
}
Context::~Context()
{
    if (!GPU) return;
    Flush(true); SDL_WaitForGPUIdle(GPU);
    for(auto* shader:{VS,PS,PresentVS,PresentPS})if(shader)SDL_ReleaseGPUShader(GPU,shader);
    if(Claimed)SDL_ReleaseWindowFromGPUDevice(GPU,Window);
    SDL_DestroyGPUDevice(GPU);
}
bool Context::Initialize()
{
    GPU=SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL|SDL_GPU_SHADERFORMAT_SPIRV,false,nullptr);
    if (!GPU) return false;
    Spirv=(SDL_GetGPUShaderFormats(GPU)&SDL_GPU_SHADERFORMAT_SPIRV)!=0;
    auto create=[&](std::span<const unsigned char> dxil,std::span<const unsigned char> spirv,SDL_GPUShaderStage stage,unsigned samplers,unsigned uniforms,const char* entry) {
        auto data=Spirv?spirv:dxil;
        SDL_GPUShaderCreateInfo info{}; info.code_size=data.size();info.code=data.data();info.entrypoint=entry;
        info.format=Spirv?SDL_GPU_SHADERFORMAT_SPIRV:SDL_GPU_SHADERFORMAT_DXIL;info.stage=stage;info.num_samplers=samplers;info.num_uniform_buffers=uniforms;
        return SDL_CreateGPUShader(GPU,&info);
    };
    VS=create(FixedVertexDxil,FixedVertexSpirv,SDL_GPU_SHADERSTAGE_VERTEX,0,1,"main");
    PS=create(FixedFragmentDxil,FixedFragmentSpirv,SDL_GPU_SHADERSTAGE_FRAGMENT,8,1,"main");
    PresentVS=create(PresentVertexDxil,PresentVertexSpirv,SDL_GPU_SHADERSTAGE_VERTEX,0,0,"VertexMain");
    PresentPS=create(PresentFragmentDxil,PresentFragmentSpirv,SDL_GPU_SHADERSTAGE_FRAGMENT,2,0,"FragmentMain");
    for(unsigned i=0;i<Palette.size();++i)Palette[i]={static_cast<BYTE>(i),static_cast<BYTE>(i),static_cast<BYTE>(i),255};
    SDL_Log("SDL GPU renderer: %s",SDL_GetGPUDeviceDriver(GPU));
    return VS && PS && PresentVS && PresentPS;
}

template<class T> void Bind(T*& destination,T* source)
{
    if(source)source->AddRef();
    if(destination)destination->Release();
    destination=source;
}
template<class T> HRESULT Return(T* value,T** output)
{
    if(!output)return D3DERR_INVALIDCALL;
    *output=value;if(value)value->AddRef();return D3D_OK;
}
unsigned VertexBytes(DWORD fvf)
{
    unsigned position=fvf&D3DFVF_POSITION_MASK;
    if(position!=D3DFVF_XYZ && position!=D3DFVF_XYZRHW)return 0;
    unsigned bytes=position==D3DFVF_XYZ?12:16;
    if(fvf&D3DFVF_NORMAL)bytes+=12;
    if(fvf&D3DFVF_PSIZE)bytes+=4;
    if(fvf&D3DFVF_DIFFUSE)bytes+=4;
    if(fvf&D3DFVF_SPECULAR)bytes+=4;
    unsigned count=(fvf&D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT;
    if(count>8)return 0;
    constexpr unsigned dimensions[]={2,3,4,1};
    for(unsigned i=0;i<count;++i)bytes+=4*dimensions[(fvf>>(16+2*i))&3];
    return bytes;
}
bool PrepareVertices(BufferData& data,DWORD fvf,unsigned stride)
{
    unsigned bytes=VertexBytes(fvf);
    if(!bytes || stride<bytes || data.Locked)return false;
    if(!data.Dirty && data.GPU && data.GPUFVF==fvf && data.GPUStride==stride)return true;
    std::vector<Vertex> vertices(data.Bytes.size()/stride);
    constexpr unsigned dimensions[]={2,3,4,1};
    for(size_t i=0;i<vertices.size();++i){
        auto* p=data.Bytes.data()+i*stride;auto& v=vertices[i];
        auto read=[&](void* out,unsigned count){std::memcpy(out,p,count);p+=count;};
        read(&v.Position,(fvf&D3DFVF_POSITION_MASK)==D3DFVF_XYZ?12:16);
        if(fvf&D3DFVF_NORMAL)read(&v.Normal,12);
        if(fvf&D3DFVF_PSIZE)p+=4;
        DWORD color;
        if(fvf&D3DFVF_DIFFUSE){read(&color,4);v.Diffuse=Color(color);}
        if(fvf&D3DFVF_SPECULAR){read(&color,4);v.Specular=Color(color);}
        for(auto& uv:v.Coordinates)uv.w=1;
        unsigned count=(fvf&D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT;
        for(unsigned t=0;t<count;++t)read(&v.Coordinates[t],4*dimensions[(fvf>>(16+2*t))&3]);
    }
    unsigned size=static_cast<unsigned>(vertices.size()*sizeof(Vertex));
    if(!size)return false;
    if(data.GPU && data.GPUSize!=size){SDL_ReleaseGPUBuffer(data.Ctx->GPU,data.GPU);data.GPU=nullptr;}
    data.GPU=data.Ctx->UploadBuffer(vertices.data(),size,SDL_GPU_BUFFERUSAGE_VERTEX,data.GPU);
    if(!data.GPU)return false;
    data.GPUSize=size;data.GPUFVF=fvf;data.GPUStride=stride;data.Dirty=false;return true;
}
bool PrepareIndices(BufferData& data)
{
    if(data.Locked)return false;
    if(!data.Dirty && data.GPU)return true;
    data.GPU=data.Ctx->UploadBuffer(data.Bytes.data(),static_cast<unsigned>(data.Bytes.size()),SDL_GPU_BUFFERUSAGE_INDEX,data.GPU);
    if(!data.GPU)return false;
    data.Dirty=false;return true;
}
void VertexBuffer::PreLoad()
{
    unsigned stride=VertexBytes(Data.FVF);
    if(stride){if(!PrepareVertices(Data,Data.FVF,stride))Failure("Preload vertices");}
    else{
        if(Data.GPU)SDL_ReleaseGPUBuffer(Ctx->GPU,Data.GPU);
        Data.GPU=Ctx->UploadBuffer(Data.Bytes.data(),static_cast<unsigned>(Data.Bytes.size()),SDL_GPU_BUFFERUSAGE_VERTEX);
        Data.GPUSize=static_cast<unsigned>(Data.Bytes.size());Data.GPUFVF=Data.GPUStride=0;Data.Dirty=true;
        if(!Data.GPU)Failure("Preload vertices");
    }
}
void IndexBuffer::PreLoad() {if(!PrepareIndices(Data))Failure("Preload indices");}
unsigned ElementCount(D3DPRIMITIVETYPE type,unsigned count)
{
    switch(type){
    case D3DPT_POINTLIST:return count;
    case D3DPT_LINELIST:return count*2;
    case D3DPT_LINESTRIP:return count+1;
    case D3DPT_TRIANGLELIST:return count*3;
    case D3DPT_TRIANGLESTRIP:case D3DPT_TRIANGLEFAN:return count+2;
    default:return 0;
    }
}
SDL_GPUPrimitiveType Topology(D3DPRIMITIVETYPE type)
{
    switch(type){
    case D3DPT_POINTLIST:return SDL_GPU_PRIMITIVETYPE_POINTLIST;
    case D3DPT_LINELIST:return SDL_GPU_PRIMITIVETYPE_LINELIST;
    case D3DPT_LINESTRIP:return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
    case D3DPT_TRIANGLESTRIP:return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    default:return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    }
}
SDL_GPUCompareOp Compare(DWORD value)
{
    constexpr SDL_GPUCompareOp ops[]={SDL_GPU_COMPAREOP_INVALID,SDL_GPU_COMPAREOP_NEVER,SDL_GPU_COMPAREOP_LESS,
        SDL_GPU_COMPAREOP_EQUAL,SDL_GPU_COMPAREOP_LESS_OR_EQUAL,SDL_GPU_COMPAREOP_GREATER,
        SDL_GPU_COMPAREOP_NOT_EQUAL,SDL_GPU_COMPAREOP_GREATER_OR_EQUAL,SDL_GPU_COMPAREOP_ALWAYS};
    return value>=1 && value<=8?ops[value]:SDL_GPU_COMPAREOP_ALWAYS;
}
SDL_GPUStencilOp Stencil(DWORD value)
{
    constexpr SDL_GPUStencilOp ops[]={SDL_GPU_STENCILOP_INVALID,SDL_GPU_STENCILOP_KEEP,SDL_GPU_STENCILOP_ZERO,
        SDL_GPU_STENCILOP_REPLACE,SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP,SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP,
        SDL_GPU_STENCILOP_INVERT,SDL_GPU_STENCILOP_INCREMENT_AND_WRAP,SDL_GPU_STENCILOP_DECREMENT_AND_WRAP};
    return value>=1 && value<=8?ops[value]:SDL_GPU_STENCILOP_KEEP;
}
SDL_GPUBlendFactor Blend(DWORD value)
{
    constexpr SDL_GPUBlendFactor factors[]={SDL_GPU_BLENDFACTOR_INVALID,SDL_GPU_BLENDFACTOR_ZERO,SDL_GPU_BLENDFACTOR_ONE,
        SDL_GPU_BLENDFACTOR_SRC_COLOR,SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR,SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,SDL_GPU_BLENDFACTOR_DST_ALPHA,SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
        SDL_GPU_BLENDFACTOR_DST_COLOR,SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR,SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE};
    return value>=1 && value<=11?factors[value]:SDL_GPU_BLENDFACTOR_ONE;
}
SDL_GPUBlendOp BlendOperation(DWORD value)
{
    constexpr SDL_GPUBlendOp ops[]={SDL_GPU_BLENDOP_INVALID,SDL_GPU_BLENDOP_ADD,SDL_GPU_BLENDOP_SUBTRACT,
        SDL_GPU_BLENDOP_REVERSE_SUBTRACT,SDL_GPU_BLENDOP_MIN,SDL_GPU_BLENDOP_MAX};
    return value>=1 && value<=5?ops[value]:SDL_GPU_BLENDOP_ADD;
}
SDL_GPUSamplerAddressMode Address(DWORD value)
{
    if(value==D3DTADDRESS_WRAP)return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    if(value==D3DTADDRESS_MIRROR)return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
}
struct Device final : Object<IDirect3DDevice8> {
    std::shared_ptr<Context> Ctx;
    IDirect3D8* Parent;
    D3DPRESENT_PARAMETERS Parameters{};
    D3DDEVICE_CREATION_PARAMETERS Creation{};
    std::array<DWORD,256> States{};
    std::array<std::array<DWORD,33>,8> Stages{};
    std::map<unsigned,Matrix> Matrices;
    std::array<D3DLIGHT8,8> Lights{};
    std::array<BOOL,8> LightEnabled{};
    D3DMATERIAL8 Material{};
    D3DVIEWPORT8 Viewport{};
    std::array<Texture*,8> Textures{};
    VertexBuffer* Vertices=nullptr;
    IndexBuffer* Indices=nullptr;
    Surface *Backbuffer=nullptr,*Depth=nullptr,*Target=nullptr,*TargetDepth=nullptr;
    unsigned Stride=0,BaseVertex=0;
    DWORD FVF=D3DFVF_XYZ;
    bool Scene=false,GammaDirty=true;
    D3DGAMMARAMP Gamma{};
    SDL_GPUTexture* GammaTexture=nullptr;
    std::shared_ptr<Image> White;
    SDL_GPUGraphicsPipeline* PresentPipeline=nullptr;
    SDL_GPUSampler* PresentSampler=nullptr;
    std::map<std::vector<DWORD>,SDL_GPUGraphicsPipeline*> Pipelines;
    std::map<std::array<DWORD,10>,SDL_GPUSampler*> Samplers;
    std::map<UINT,std::array<PALETTEENTRY,256>> Palettes;
    UINT Palette=0;
    Device(std::shared_ptr<Context> ctx,IDirect3D8* parent,D3DDEVICE_CREATION_PARAMETERS creation);
    ~Device() override;
    Matrix& MatrixAt(unsigned state){return Matrices.try_emplace(state,Identity()).first->second;}
    bool Pass(DWORD flags=0,D3DCOLOR color=0,float z=1,DWORD stencil=0);
    bool Uniforms();
    SDL_GPUGraphicsPipeline* Pipeline(D3DPRIMITIVETYPE type);
    SDL_GPUSampler* Sampler(unsigned stage);
    bool UploadGamma();
    HRESULT Draw(D3DPRIMITIVETYPE type,BufferData& vertices,unsigned stride,BufferData* indices,unsigned start,unsigned count,unsigned base);
    HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override {return Ctx->GPU?D3D_OK:D3DERR_DEVICELOST;}
    UINT STDMETHODCALLTYPE GetAvailableTextureMem() override {return 0;}
    HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD) override;
    HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D8** output) override {return Return(Parent,output);}
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8* output) override {return Parent->GetDeviceCaps(0,D3DDEVTYPE_HAL,output);}
    HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* output) override {if(!output)return D3DERR_INVALIDCALL;*output={Parameters.BackBufferWidth,Parameters.BackBufferHeight,Parameters.FullScreen_RefreshRateInHz,Parameters.BackBufferFormat};return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* output) override {if(!output)return D3DERR_INVALIDCALL;*output=Creation;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT,UINT,IDirect3DSurface8*) override {return Unsupported("SetCursorProperties");}
    void STDMETHODCALLTYPE SetCursorPosition(int x,int y,DWORD) override {SDL_WarpMouseInWindow(Ctx->Window,static_cast<float>(x),static_cast<float>(y));}
    BOOL STDMETHODCALLTYPE ShowCursor(BOOL show) override {BOOL old=SDL_CursorVisible();if(show)SDL_ShowCursor();else SDL_HideCursor();return old;}
    HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS*,IDirect3DSwapChain8**) override;
    HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS*) override;
    HRESULT STDMETHODCALLTYPE Present(const RECT*,const RECT*,HWND,const RGNDATA*) override;
    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT index,D3DBACKBUFFER_TYPE type,IDirect3DSurface8** output) override {if(index || type!=D3DBACKBUFFER_TYPE_MONO)return D3DERR_INVALIDCALL;return Return<IDirect3DSurface8>(Backbuffer,output);}
    HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS*) override {return Unsupported("GetRasterStatus");}
    void STDMETHODCALLTYPE SetGammaRamp(DWORD,const D3DGAMMARAMP* ramp) override {if(ramp){Gamma=*ramp;GammaDirty=true;}}
    void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP* ramp) override {if(ramp)*ramp=Gamma;}
    HRESULT STDMETHODCALLTYPE CreateTexture(UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DTexture8**) override;
    HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT,UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DVolumeTexture8**) override {return Unsupported("CreateVolumeTexture");}
    HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DCubeTexture8**) override {return Unsupported("CreateCubeTexture");}
    HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT length,DWORD usage,DWORD fvf,D3DPOOL pool,IDirect3DVertexBuffer8** output) override {if(!output || !length)return D3DERR_INVALIDCALL;*output=new VertexBuffer(Ctx,length,usage,fvf,pool);return D3D_OK;}
    HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT length,DWORD usage,D3DFORMAT format,D3DPOOL pool,IDirect3DIndexBuffer8** output) override {if(!output || !length || (format!=D3DFMT_INDEX16 && format!=D3DFMT_INDEX32))return D3DERR_INVALIDCALL;*output=new IndexBuffer(Ctx,length,usage,format,pool);return D3D_OK;}
    HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT,UINT,D3DFORMAT,D3DMULTISAMPLE_TYPE,BOOL,IDirect3DSurface8**) override;
    HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT,UINT,D3DFORMAT,D3DMULTISAMPLE_TYPE,IDirect3DSurface8**) override;
    HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT,UINT,D3DFORMAT,IDirect3DSurface8**) override;
    HRESULT STDMETHODCALLTYPE CopyRects(IDirect3DSurface8*,const RECT*,UINT,IDirect3DSurface8*,const POINT*) override;
    HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture8*,IDirect3DBaseTexture8*) override;
    HRESULT STDMETHODCALLTYPE GetFrontBuffer(IDirect3DSurface8* output) override {return CopyRects(Backbuffer,nullptr,0,output,nullptr);}
    HRESULT STDMETHODCALLTYPE SetRenderTarget(IDirect3DSurface8*,IDirect3DSurface8*) override;
    HRESULT STDMETHODCALLTYPE GetRenderTarget(IDirect3DSurface8** output) override {return Return<IDirect3DSurface8>(Target,output);}
    HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface8** output) override {return Return<IDirect3DSurface8>(TargetDepth,output);}
    HRESULT STDMETHODCALLTYPE BeginScene() override {if(Scene)return D3DERR_INVALIDCALL;Scene=true;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE EndScene() override {if(!Scene)return D3DERR_INVALIDCALL;Ctx->EndPass();Scene=false;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE Clear(DWORD count,const D3DRECT*,DWORD flags,D3DCOLOR color,float z,DWORD stencil) override {if(count)return Unsupported("Clear rectangles");Ctx->EndPass();return Pass(flags,color,z,stencil)?D3D_OK:Failure("Clear");}
    HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE state,const D3DMATRIX* matrix) override {if(!matrix)return D3DERR_INVALIDCALL;std::memcpy(MatrixAt(state).data(),matrix,64);return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE state,D3DMATRIX* matrix) override {if(!matrix)return D3DERR_INVALIDCALL;std::memcpy(matrix,MatrixAt(state).data(),64);return D3D_OK;}
    HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE state,const D3DMATRIX* matrix) override {if(!matrix)return D3DERR_INVALIDCALL;Matrix m;std::memcpy(m.data(),matrix,64);MatrixAt(state)=Multiply(m,MatrixAt(state));return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetViewport(const D3DVIEWPORT8* viewport) override {if(!viewport || !viewport->Width || !viewport->Height)return D3DERR_INVALIDCALL;Viewport=*viewport;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8* output) override {if(!output)return D3DERR_INVALIDCALL;*output=Viewport;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetMaterial(const D3DMATERIAL8* material) override {if(!material)return D3DERR_INVALIDCALL;Material=*material;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8* output) override {if(!output)return D3DERR_INVALIDCALL;*output=Material;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetLight(DWORD index,const D3DLIGHT8* light) override {if(!light || index>=8)return D3DERR_INVALIDCALL;Lights[index]=*light;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetLight(DWORD index,D3DLIGHT8* output) override {if(!output || index>=8)return D3DERR_INVALIDCALL;*output=Lights[index];return D3D_OK;}
    HRESULT STDMETHODCALLTYPE LightEnable(DWORD index,BOOL enable) override {if(index>=8)return D3DERR_INVALIDCALL;LightEnabled[index]=enable;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD index,BOOL* output) override {if(!output || index>=8)return D3DERR_INVALIDCALL;*output=LightEnabled[index];return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD,const float*) override {return Unsupported("SetClipPlane");}
    HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD,float*) override {return Unsupported("GetClipPlane");}
    HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE state,DWORD value) override {if(static_cast<unsigned>(state)>=States.size())return D3DERR_INVALIDCALL;States[state]=value;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE state,DWORD* output) override {if(!output || static_cast<unsigned>(state)>=States.size())return D3DERR_INVALIDCALL;*output=States[state];return D3D_OK;}
    HRESULT STDMETHODCALLTYPE BeginStateBlock() override {return Unsupported("BeginStateBlock");}
    HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD*) override {return Unsupported("EndStateBlock");}
    HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD) override {return Unsupported("ApplyStateBlock");}
    HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD) override {return Unsupported("CaptureStateBlock");}
    HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD) override {return Unsupported("DeleteStateBlock");}
    HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE,DWORD*) override {return Unsupported("CreateStateBlock");}
    HRESULT STDMETHODCALLTYPE SetClipStatus(const D3DCLIPSTATUS8*) override {return Unsupported("SetClipStatus");}
    HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8*) override {return Unsupported("GetClipStatus");}
    HRESULT STDMETHODCALLTYPE GetTexture(DWORD stage,IDirect3DBaseTexture8** output) override {if(stage>=8)return D3DERR_INVALIDCALL;return Return<IDirect3DBaseTexture8>(Textures[stage],output);}
    HRESULT STDMETHODCALLTYPE SetTexture(DWORD stage,IDirect3DBaseTexture8* texture) override {if(stage>=8 || (texture && !dynamic_cast<Texture*>(texture)))return D3DERR_INVALIDCALL;Bind(Textures[stage],static_cast<Texture*>(texture));return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD stage,D3DTEXTURESTAGESTATETYPE state,DWORD* output) override {if(!output || stage>=8 || static_cast<unsigned>(state)>=33)return D3DERR_INVALIDCALL;*output=Stages[stage][state];return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD stage,D3DTEXTURESTAGESTATETYPE state,DWORD value) override {if(stage>=8 || static_cast<unsigned>(state)>=33)return D3DERR_INVALIDCALL;Stages[stage][state]=value;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* passes) override {if(!passes)return D3DERR_INVALIDCALL;*passes=1;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetInfo(DWORD,void*,DWORD) override {return Unsupported("GetInfo");}
    HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT number,const PALETTEENTRY* entries) override {if(!entries)return D3DERR_INVALIDCALL;std::memcpy(Palettes[number].data(),entries,sizeof(PALETTEENTRY)*256);if(number==Palette){Ctx->Palette=Palettes[number];++Ctx->PaletteVersion;}return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT number,PALETTEENTRY* entries) override {if(!entries || !Palettes.contains(number))return D3DERR_INVALIDCALL;std::memcpy(entries,Palettes[number].data(),sizeof(PALETTEENTRY)*256);return D3D_OK;}
    HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT number) override {if(!Palettes.contains(number))return D3DERR_INVALIDCALL;Palette=number;Ctx->Palette=Palettes[number];++Ctx->PaletteVersion;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT* output) override {if(!output)return D3DERR_INVALIDCALL;*output=Palette;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE type,UINT start,UINT count) override {if(!Vertices)return D3DERR_INVALIDCALL;return Draw(type,Vertices->Data,Stride,nullptr,start,count,0);}
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE type,UINT,UINT,UINT start,UINT count) override {if(!Vertices || !Indices)return D3DERR_INVALIDCALL;return Draw(type,Vertices->Data,Stride,&Indices->Data,start,count,BaseVertex);}
    HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE,UINT,const void*,UINT) override;
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE,UINT,UINT,UINT,const void*,D3DFORMAT,const void*,UINT) override;
    HRESULT STDMETHODCALLTYPE ProcessVertices(UINT,UINT,UINT,IDirect3DVertexBuffer8*,DWORD) override {return Unsupported("ProcessVertices");}
    HRESULT STDMETHODCALLTYPE CreateVertexShader(const DWORD*,const DWORD*,DWORD*,DWORD) override {return Unsupported("CreateVertexShader");}
    HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD handle) override {if(!VertexBytes(handle))return Unsupported("SetVertexShader program");FVF=handle;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD* output) override {if(!output)return D3DERR_INVALIDCALL;*output=FVF;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD) override {return Unsupported("DeleteVertexShader");}
    HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD,const void*,DWORD) override {return Unsupported("SetVertexShaderConstant");}
    HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD,void*,DWORD) override {return Unsupported("GetVertexShaderConstant");}
    HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD,void*,DWORD*) override {return Unsupported("GetVertexShaderDeclaration");}
    HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD,void*,DWORD*) override {return Unsupported("GetVertexShaderFunction");}
    HRESULT STDMETHODCALLTYPE SetStreamSource(UINT stream,IDirect3DVertexBuffer8* buffer,UINT stride) override {if(stream || (buffer && !dynamic_cast<VertexBuffer*>(buffer)))return D3DERR_INVALIDCALL;Bind(Vertices,static_cast<VertexBuffer*>(buffer));Stride=stride;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetStreamSource(UINT stream,IDirect3DVertexBuffer8** output,UINT* stride) override {if(stream || !stride)return D3DERR_INVALIDCALL;*stride=Stride;return Return<IDirect3DVertexBuffer8>(Vertices,output);}
    HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer8* buffer,UINT base) override {if(buffer && !dynamic_cast<IndexBuffer*>(buffer))return D3DERR_INVALIDCALL;Bind(Indices,static_cast<IndexBuffer*>(buffer));BaseVertex=base;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer8** output,UINT* base) override {if(!base)return D3DERR_INVALIDCALL;*base=BaseVertex;return Return<IDirect3DIndexBuffer8>(Indices,output);}
    HRESULT STDMETHODCALLTYPE CreatePixelShader(const DWORD*,DWORD*) override {return Unsupported("CreatePixelShader");}
    HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD handle) override {return handle?Unsupported("SetPixelShader program"):D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD* output) override {if(!output)return D3DERR_INVALIDCALL;*output=0;return D3D_OK;}
    HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD) override {return Unsupported("DeletePixelShader");}
    HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD,const void*,DWORD) override {return Unsupported("SetPixelShaderConstant");}
    HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD,void*,DWORD) override {return Unsupported("GetPixelShaderConstant");}
    HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD,void*,DWORD*) override {return Unsupported("GetPixelShaderFunction");}
    HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT,const float*,const D3DRECTPATCH_INFO*) override {return Unsupported("DrawRectPatch");}
    HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT,const float*,const D3DTRIPATCH_INFO*) override {return Unsupported("DrawTriPatch");}
    HRESULT STDMETHODCALLTYPE DeletePatch(UINT) override {return Unsupported("DeletePatch");}
};
HRESULT GetOwner(Context& context,IDirect3DDevice8** output)
{
    if(!output)return D3DERR_INVALIDCALL;
    *output=nullptr;if(!context.Owner)return D3DERR_DEVICELOST;
    return Return<IDirect3DDevice8>(context.Owner,output);
}
Device::Device(std::shared_ptr<Context> ctx,IDirect3D8* parent,D3DDEVICE_CREATION_PARAMETERS creation)
    :Ctx(std::move(ctx)),Parent(parent),Creation(creation)
{
    Parent->AddRef();Ctx->Owner=this;
    States[D3DRS_ZENABLE]=D3DZB_TRUE;States[D3DRS_ZWRITEENABLE]=TRUE;
    States[D3DRS_FILLMODE]=D3DFILL_SOLID;States[D3DRS_SHADEMODE]=D3DSHADE_GOURAUD;
    States[D3DRS_SRCBLEND]=D3DBLEND_ONE;States[D3DRS_DESTBLEND]=D3DBLEND_ZERO;
    States[D3DRS_CULLMODE]=D3DCULL_CCW;States[D3DRS_ZFUNC]=D3DCMP_LESSEQUAL;States[D3DRS_ALPHAFUNC]=D3DCMP_ALWAYS;
    States[D3DRS_FOGEND]=States[D3DRS_FOGDENSITY]=std::bit_cast<DWORD>(1.0f);
    States[D3DRS_STENCILFAIL]=States[D3DRS_STENCILZFAIL]=States[D3DRS_STENCILPASS]=D3DSTENCILOP_KEEP;
    States[D3DRS_STENCILFUNC]=D3DCMP_ALWAYS;States[D3DRS_STENCILMASK]=States[D3DRS_STENCILWRITEMASK]=0xffffffff;
    States[D3DRS_TEXTUREFACTOR]=0xffffffff;States[D3DRS_CLIPPING]=TRUE;States[D3DRS_LIGHTING]=TRUE;
    States[D3DRS_COLORVERTEX]=TRUE;States[D3DRS_LOCALVIEWER]=TRUE;
    States[D3DRS_DIFFUSEMATERIALSOURCE]=D3DMCS_COLOR1;States[D3DRS_SPECULARMATERIALSOURCE]=D3DMCS_COLOR2;
    States[D3DRS_COLORWRITEENABLE]=15;States[D3DRS_BLENDOP]=D3DBLENDOP_ADD;
    for(unsigned i=0;i<8;++i){auto& s=Stages[i];
        s[D3DTSS_COLOROP]=i?D3DTOP_DISABLE:D3DTOP_MODULATE;s[D3DTSS_ALPHAOP]=i?D3DTOP_DISABLE:D3DTOP_SELECTARG1;
        s[D3DTSS_COLORARG1]=s[D3DTSS_ALPHAARG1]=D3DTA_TEXTURE;s[D3DTSS_COLORARG2]=s[D3DTSS_ALPHAARG2]=D3DTA_CURRENT;
        s[D3DTSS_COLORARG0]=s[D3DTSS_ALPHAARG0]=D3DTA_CURRENT;s[D3DTSS_RESULTARG]=D3DTA_CURRENT;
        s[D3DTSS_TEXCOORDINDEX]=i;s[D3DTSS_ADDRESSU]=s[D3DTSS_ADDRESSV]=s[D3DTSS_ADDRESSW]=D3DTADDRESS_WRAP;
        s[D3DTSS_MINFILTER]=s[D3DTSS_MAGFILTER]=D3DTEXF_POINT;s[D3DTSS_MAXANISOTROPY]=1;
    }
    Material.Diffuse=Material.Ambient={1,1,1,1};
    for(unsigned i=0;i<256;++i)Gamma.red[i]=Gamma.green[i]=Gamma.blue[i]=static_cast<WORD>(i*257);
    White=std::make_shared<Image>(Ctx,1,1,1,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,0);
    std::fill(White->Levels[0].Bytes.begin(),White->Levels[0].Bytes.end(),255);
    Palettes[0]=Ctx->Palette;
}
Device::~Device()
{
    Ctx->Flush(true);SDL_WaitForGPUIdle(Ctx->GPU);
    for(auto*& t:Textures)Bind<Texture>(t,nullptr);
    Bind<VertexBuffer>(Vertices,nullptr);Bind<IndexBuffer>(Indices,nullptr);
    Bind<Surface>(Target,nullptr);Bind<Surface>(TargetDepth,nullptr);Bind<Surface>(Backbuffer,nullptr);Bind<Surface>(Depth,nullptr);
    for(auto& [key,pipeline]:Pipelines)SDL_ReleaseGPUGraphicsPipeline(Ctx->GPU,pipeline);
    for(auto& [key,sampler]:Samplers)SDL_ReleaseGPUSampler(Ctx->GPU,sampler);
    if(PresentPipeline)SDL_ReleaseGPUGraphicsPipeline(Ctx->GPU,PresentPipeline);
    if(PresentSampler)SDL_ReleaseGPUSampler(Ctx->GPU,PresentSampler);
    if(GammaTexture)SDL_ReleaseGPUTexture(Ctx->GPU,GammaTexture);
    Ctx->Owner=nullptr;Parent->Release();
}
HRESULT Device::CreateTexture(UINT width,UINT height,UINT levels,DWORD usage,D3DFORMAT format,D3DPOOL pool,IDirect3DTexture8** output)
{
    if(!output || !width || !height || width>4096 || height>4096 || (!BytesPerPixel(format) && !BlockBytes(format)) || DepthFormat(format))return D3DERR_INVALIDCALL;
    auto image=std::make_shared<Image>(Ctx,width,height,levels,format,pool,usage);
    if(!Ctx->CreateImage(*image))return Failure("Create texture");
    if(pool==D3DPOOL_MANAGED)Ctx->ManagedImages.push_back(image);
    *output=new Texture(std::move(image));return D3D_OK;
}
HRESULT Device::CreateRenderTarget(UINT width,UINT height,D3DFORMAT format,D3DMULTISAMPLE_TYPE samples,BOOL,IDirect3DSurface8** output)
{
    if(!output || !width || !height || samples!=D3DMULTISAMPLE_NONE || !BytesPerPixel(format) || DepthFormat(format))return D3DERR_INVALIDCALL;
    auto image=std::make_shared<Image>(Ctx,width,height,1,format,D3DPOOL_DEFAULT,D3DUSAGE_RENDERTARGET);
    if(!Ctx->CreateImage(*image))return Failure("Create render target");
    *output=new Surface(std::move(image));return D3D_OK;
}
HRESULT Device::CreateDepthStencilSurface(UINT width,UINT height,D3DFORMAT format,D3DMULTISAMPLE_TYPE samples,IDirect3DSurface8** output)
{
    if(!output || !width || !height || samples!=D3DMULTISAMPLE_NONE || !DepthFormat(format))return D3DERR_INVALIDCALL;
    auto image=std::make_shared<Image>(Ctx,width,height,1,format,D3DPOOL_DEFAULT,D3DUSAGE_DEPTHSTENCIL);
    if(!Ctx->CreateImage(*image))return Failure("Create depth surface");
    *output=new Surface(std::move(image));return D3D_OK;
}
HRESULT Device::CreateImageSurface(UINT width,UINT height,D3DFORMAT format,IDirect3DSurface8** output)
{
    if(!output || !width || !height || (!BytesPerPixel(format) && !BlockBytes(format)) || DepthFormat(format))return D3DERR_INVALIDCALL;
    *output=new Surface(std::make_shared<Image>(Ctx,width,height,1,format,D3DPOOL_SYSTEMMEM,0));return D3D_OK;
}
HRESULT Device::Reset(D3DPRESENT_PARAMETERS* parameters)
{
    if(!parameters || Scene || parameters->MultiSampleType!=D3DMULTISAMPLE_NONE)return D3DERR_INVALIDCALL;
    if(!Ctx->Flush(true))return Failure("Reset submit");
    SDL_WaitForGPUIdle(Ctx->GPU);
    Parameters=*parameters;
    int width=0,height=0;SDL_GetWindowSizeInPixels(Ctx->Window,&width,&height);
    if(!Parameters.BackBufferWidth)Parameters.BackBufferWidth=width;
    if(!Parameters.BackBufferHeight)Parameters.BackBufferHeight=height;
    if(Parameters.BackBufferFormat==D3DFMT_UNKNOWN)Parameters.BackBufferFormat=D3DFMT_X8R8G8B8;
    SDL_GPUPresentMode mode=Parameters.FullScreen_PresentationInterval==D3DPRESENT_INTERVAL_IMMEDIATE?SDL_GPU_PRESENTMODE_IMMEDIATE:SDL_GPU_PRESENTMODE_VSYNC;
    if(!SDL_WindowSupportsGPUPresentMode(Ctx->GPU,Ctx->Window,mode))mode=SDL_GPU_PRESENTMODE_VSYNC;
    if(!SDL_SetGPUSwapchainParameters(Ctx->GPU,Ctx->Window,SDL_GPU_SWAPCHAINCOMPOSITION_SDR,mode))return Failure("Configure swapchain");
    IDirect3DSurface8* back=nullptr;
    HRESULT result=CreateRenderTarget(Parameters.BackBufferWidth,Parameters.BackBufferHeight,Parameters.BackBufferFormat,D3DMULTISAMPLE_NONE,TRUE,&back);
    if(FAILED(result))return result;
    IDirect3DSurface8* depth=nullptr;
    if(Parameters.EnableAutoDepthStencil){result=CreateDepthStencilSurface(Parameters.BackBufferWidth,Parameters.BackBufferHeight,Parameters.AutoDepthStencilFormat,D3DMULTISAMPLE_NONE,&depth);if(FAILED(result)){back->Release();return result;}}
    Bind<Surface>(Target,nullptr);Bind<Surface>(TargetDepth,nullptr);
    Bind(Backbuffer,static_cast<Surface*>(back));back->Release();
    Bind(Depth,static_cast<Surface*>(depth));if(depth)depth->Release();
    Bind(Target,Backbuffer);Bind(TargetDepth,Depth);
    Viewport={0,0,Parameters.BackBufferWidth,Parameters.BackBufferHeight,0,1};
    *parameters=Parameters;return D3D_OK;
}
bool Device::Pass(DWORD flags,D3DCOLOR color,float z,DWORD stencil)
{
    if(Ctx->Pass)return true;
    if(!Target || !Ctx->Command())return false;
    auto& image=*Target->Data;
    if(!image.GPU && !Ctx->CreateImage(image))return false;
    if(!(flags&D3DCLEAR_TARGET) && !Ctx->Upload(image))return false;
    if(TargetDepth && !TargetDepth->Data->GPU && !Ctx->CreateImage(*TargetDepth->Data))return false;
    SDL_GPUColorTargetInfo target{};target.texture=image.GPU;target.mip_level=Target->Level;
    Float4 c=Color(color);target.clear_color={c.x,c.y,c.z,c.w};
    target.load_op=flags&D3DCLEAR_TARGET?SDL_GPU_LOADOP_CLEAR:SDL_GPU_LOADOP_LOAD;target.store_op=SDL_GPU_STOREOP_STORE;
    SDL_GPUDepthStencilTargetInfo depth{};
    if(TargetDepth){
        depth.texture=TargetDepth->Data->GPU;depth.mip_level=static_cast<Uint8>(TargetDepth->Level);
        depth.clear_depth=z;depth.clear_stencil=static_cast<Uint8>(stencil);
        depth.load_op=flags&D3DCLEAR_ZBUFFER?SDL_GPU_LOADOP_CLEAR:SDL_GPU_LOADOP_LOAD;depth.store_op=SDL_GPU_STOREOP_STORE;
        depth.stencil_load_op=flags&D3DCLEAR_STENCIL?SDL_GPU_LOADOP_CLEAR:SDL_GPU_LOADOP_LOAD;depth.stencil_store_op=SDL_GPU_STOREOP_STORE;
    }
    Ctx->Pass=SDL_BeginGPURenderPass(Ctx->Commands,&target,1,TargetDepth?&depth:nullptr);
    if(!Ctx->Pass)return false;
    auto& level=image.Levels[Target->Level];level.GPUWritten=true;level.Dirty=false;
    return true;
}
HRESULT Device::SetRenderTarget(IDirect3DSurface8* target,IDirect3DSurface8* depth)
{
    auto* t=dynamic_cast<Surface*>(target);auto* d=dynamic_cast<Surface*>(depth);
    if((target && !t) || (depth && !d) || (t && !(t->Data->Usage&D3DUSAGE_RENDERTARGET)) || (d && !DepthFormat(d->Data->Format)))return D3DERR_INVALIDCALL;
    Ctx->EndPass();if(t)Bind(Target,t);Bind(TargetDepth,d);
    if(Target){auto& l=Target->Data->Levels[Target->Level];Viewport={0,0,l.Width,l.Height,0,1};}
    return D3D_OK;
}
HRESULT Device::CopyRects(IDirect3DSurface8* source,const RECT* rects,UINT count,IDirect3DSurface8* destination,const POINT* points)
{
    auto* s=dynamic_cast<Surface*>(source);auto* d=dynamic_cast<Surface*>(destination);
    if(!s || !d || s->Data->Format!=d->Data->Format)return D3DERR_INVALIDCALL;
    auto& from=s->Data->Levels[s->Level];auto& to=d->Data->Levels[d->Level];
    if(from.GPUWritten && !Ctx->ReadImage(*s->Data,s->Level))return Failure("Read copy source");
    if(to.GPUWritten && !Ctx->ReadImage(*d->Data,d->Level))return Failure("Read copy destination");
    if(!rects){if(count)return D3DERR_INVALIDCALL;RECT rect{0,0,static_cast<LONG>(from.Width),static_cast<LONG>(from.Height)};POINT point{};return CopyRects(source,&rect,1,destination,&point);}
    unsigned block=BlockBytes(s->Data->Format),pixel=BytesPerPixel(s->Data->Format);
    for(unsigned i=0;i<count;++i){auto& r=rects[i];POINT p=points?points[i]:POINT{r.left,r.top};
        if(r.left<0 || r.top<0 || r.right<=r.left || r.bottom<=r.top || r.right>static_cast<LONG>(from.Width) || r.bottom>static_cast<LONG>(from.Height) || p.x<0 || p.y<0 || p.x+r.right-r.left>static_cast<LONG>(to.Width) || p.y+r.bottom-r.top>static_cast<LONG>(to.Height))return D3DERR_INVALIDCALL;
        if(block && ((r.left|r.top|p.x|p.y)&3))return D3DERR_INVALIDCALL;
        unsigned sx=block?r.left/4:r.left,sy=block?r.top/4:r.top,dx=block?p.x/4:p.x,dy=block?p.y/4:p.y;
        unsigned rows=block?(r.bottom-r.top+3)/4:r.bottom-r.top,bytes=block?((r.right-r.left+3)/4)*block:(r.right-r.left)*pixel;
        for(unsigned y=0;y<rows;++y)std::memmove(to.Bytes.data()+(dy+y)*to.Pitch+dx*(block?block:pixel),from.Bytes.data()+(sy+y)*from.Pitch+sx*(block?block:pixel),bytes);
    }
    to.Dirty=true;to.GPUWritten=false;return D3D_OK;
}
HRESULT Device::UpdateTexture(IDirect3DBaseTexture8* source,IDirect3DBaseTexture8* destination)
{
    auto* s=dynamic_cast<Texture*>(source);auto* d=dynamic_cast<Texture*>(destination);
    if(!s || !d || s->Data->Format!=d->Data->Format)return D3DERR_INVALIDCALL;
    unsigned start=0;
    while(start<s->Data->Levels.size() && (s->Data->Levels[start].Width!=d->Data->Levels[0].Width || s->Data->Levels[start].Height!=d->Data->Levels[0].Height))++start;
    if(start+d->Data->Levels.size()>s->Data->Levels.size())return D3DERR_INVALIDCALL;
    for(unsigned i=0;i<d->Data->Levels.size();++i){auto& from=s->Data->Levels[start+i];auto& to=d->Data->Levels[i];
        if(from.GPUWritten && !Ctx->ReadImage(*s->Data,start+i))return Failure("Update texture source");
        to.Bytes=from.Bytes;to.Dirty=true;to.GPUWritten=false;
    }
    return D3D_OK;
}
SDL_GPUGraphicsPipeline* Device::Pipeline(D3DPRIMITIVETYPE type)
{
    constexpr unsigned keys[]={D3DRS_ZENABLE,D3DRS_ZWRITEENABLE,D3DRS_ZFUNC,D3DRS_CULLMODE,D3DRS_FILLMODE,
        D3DRS_ALPHABLENDENABLE,D3DRS_SRCBLEND,D3DRS_DESTBLEND,D3DRS_BLENDOP,D3DRS_COLORWRITEENABLE,
        D3DRS_STENCILENABLE,D3DRS_STENCILFAIL,D3DRS_STENCILZFAIL,D3DRS_STENCILPASS,D3DRS_STENCILFUNC,
        D3DRS_STENCILMASK,D3DRS_STENCILWRITEMASK,D3DRS_ZBIAS,D3DRS_CLIPPING};
    std::vector<DWORD> key{static_cast<DWORD>(Topology(type)),static_cast<DWORD>(Target->Data->ActualFormat),TargetDepth?static_cast<DWORD>(TargetDepth->Data->ActualFormat):0};
    for(unsigned state:keys)key.push_back(States[state]);
    if(auto found=Pipelines.find(key);found!=Pipelines.end())return found->second;
    SDL_GPUGraphicsPipelineCreateInfo info{};info.vertex_shader=Ctx->VS;info.fragment_shader=Ctx->PS;info.primitive_type=Topology(type);
    SDL_GPUVertexBufferDescription buffer{};buffer.slot=0;buffer.pitch=sizeof(Vertex);buffer.input_rate=SDL_GPU_VERTEXINPUTRATE_VERTEX;
    std::array<SDL_GPUVertexAttribute,12> attributes{};
    for(unsigned i=0;i<12;++i){attributes[i].location=i;attributes[i].buffer_slot=0;attributes[i].format=SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;attributes[i].offset=i*sizeof(Float4);}
    info.vertex_input_state={&buffer,1,attributes.data(),static_cast<Uint32>(attributes.size())};
    auto& raster=info.rasterizer_state;
    raster.fill_mode=States[D3DRS_FILLMODE]==D3DFILL_WIREFRAME?SDL_GPU_FILLMODE_LINE:SDL_GPU_FILLMODE_FILL;
    raster.cull_mode=States[D3DRS_CULLMODE]==D3DCULL_NONE?SDL_GPU_CULLMODE_NONE:SDL_GPU_CULLMODE_BACK;
    raster.front_face=States[D3DRS_CULLMODE]==D3DCULL_CW?SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE:SDL_GPU_FRONTFACE_CLOCKWISE;
    raster.enable_depth_clip=States[D3DRS_CLIPPING]!=0;raster.enable_depth_bias=States[D3DRS_ZBIAS]!=0;
    raster.depth_bias_constant_factor=-static_cast<float>(static_cast<int>(States[D3DRS_ZBIAS]));
    info.multisample_state.sample_count=SDL_GPU_SAMPLECOUNT_1;
    auto& depth=info.depth_stencil_state;depth.compare_op=Compare(States[D3DRS_ZFUNC]);
    depth.enable_depth_test=TargetDepth && States[D3DRS_ZENABLE]!=D3DZB_FALSE;depth.enable_depth_write=depth.enable_depth_test && States[D3DRS_ZWRITEENABLE];
    depth.enable_stencil_test=TargetDepth && HasStencil(TargetDepth->Data->ActualFormat) && States[D3DRS_STENCILENABLE];
    depth.compare_mask=static_cast<Uint8>(States[D3DRS_STENCILMASK]);depth.write_mask=static_cast<Uint8>(States[D3DRS_STENCILWRITEMASK]);
    depth.front_stencil_state={Stencil(States[D3DRS_STENCILFAIL]),Stencil(States[D3DRS_STENCILPASS]),Stencil(States[D3DRS_STENCILZFAIL]),Compare(States[D3DRS_STENCILFUNC])};
    depth.back_stencil_state=depth.front_stencil_state;
    SDL_GPUColorTargetDescription color{};color.format=Target->Data->ActualFormat;
    auto& blend=color.blend_state;blend.enable_blend=States[D3DRS_ALPHABLENDENABLE]!=0;
    DWORD src=States[D3DRS_SRCBLEND],dst=States[D3DRS_DESTBLEND];
    if(src==D3DBLEND_BOTHSRCALPHA){src=D3DBLEND_SRCALPHA;dst=D3DBLEND_INVSRCALPHA;}
    if(src==D3DBLEND_BOTHINVSRCALPHA){src=D3DBLEND_INVSRCALPHA;dst=D3DBLEND_SRCALPHA;}
    blend.src_color_blendfactor=blend.src_alpha_blendfactor=Blend(src);blend.dst_color_blendfactor=blend.dst_alpha_blendfactor=Blend(dst);
    blend.color_blend_op=blend.alpha_blend_op=BlendOperation(States[D3DRS_BLENDOP]);blend.enable_color_write_mask=true;blend.color_write_mask=States[D3DRS_COLORWRITEENABLE]&15;
    info.target_info.color_target_descriptions=&color;info.target_info.num_color_targets=1;
    info.target_info.has_depth_stencil_target=TargetDepth!=nullptr;info.target_info.depth_stencil_format=TargetDepth?TargetDepth->Data->ActualFormat:SDL_GPU_TEXTUREFORMAT_INVALID;
    auto* pipeline=SDL_CreateGPUGraphicsPipeline(Ctx->GPU,&info);
    if(pipeline)Pipelines.emplace(std::move(key),pipeline);
    return pipeline;
}
SDL_GPUSampler* Device::Sampler(unsigned stage)
{
    auto& s=Stages[stage];
    std::array<DWORD,10> key{s[D3DTSS_MINFILTER],s[D3DTSS_MAGFILTER],s[D3DTSS_MIPFILTER],s[D3DTSS_ADDRESSU],s[D3DTSS_ADDRESSV],s[D3DTSS_ADDRESSW],s[D3DTSS_MAXANISOTROPY],s[D3DTSS_MAXMIPLEVEL],Textures[stage]?Textures[stage]->LOD:0,Textures[stage]?Textures[stage]->GetLevelCount():1};
    if(auto found=Samplers.find(key);found!=Samplers.end())return found->second;
    SDL_GPUSamplerCreateInfo info{};
    info.min_filter=key[0]==D3DTEXF_POINT?SDL_GPU_FILTER_NEAREST:SDL_GPU_FILTER_LINEAR;
    info.mag_filter=key[1]==D3DTEXF_POINT?SDL_GPU_FILTER_NEAREST:SDL_GPU_FILTER_LINEAR;
    info.mipmap_mode=key[2]==D3DTEXF_LINEAR?SDL_GPU_SAMPLERMIPMAPMODE_LINEAR:SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u=Address(key[3]);info.address_mode_v=Address(key[4]);info.address_mode_w=Address(key[5]);
    info.enable_anisotropy=key[0]==D3DTEXF_ANISOTROPIC || key[1]==D3DTEXF_ANISOTROPIC;info.max_anisotropy=static_cast<float>(std::clamp(key[6],1ul,16ul));
    info.min_lod=static_cast<float>(std::max(key[7],key[8]));info.max_lod=key[2]==D3DTEXF_NONE?info.min_lod:static_cast<float>(key[9]-1);
    info.min_lod=std::min(info.min_lod,static_cast<float>(key[9]-1));info.max_lod=std::max(info.max_lod,info.min_lod);
    auto* sampler=SDL_CreateGPUSampler(Ctx->GPU,&info);if(sampler)Samplers.emplace(key,sampler);return sampler;
}
bool Device::Uniforms()
{
    VertexState v{};v.World=MatrixAt(D3DTS_WORLD);v.View=MatrixAt(D3DTS_VIEW);v.Projection=MatrixAt(D3DTS_PROJECTION);v.Normal=NormalMatrix(v.World);
    v.Viewport={static_cast<float>(Viewport.Width),static_cast<float>(Viewport.Height),static_cast<float>(Viewport.X),static_cast<float>(Viewport.Y)};
    v.Flags={static_cast<int>((FVF&D3DFVF_POSITION_MASK)==D3DFVF_XYZRHW),static_cast<int>(States[D3DRS_LIGHTING]),static_cast<int>(States[D3DRS_SPECULARENABLE]),static_cast<int>(States[D3DRS_NORMALIZENORMALS])};
    v.Diffuse=Color(Material.Diffuse);v.Ambient=Color(Material.Ambient);v.Specular=Color(Material.Specular);v.Emissive=Color(Material.Emissive);v.GlobalAmbient=Color(States[D3DRS_AMBIENT]);
    v.Settings={Material.Power,static_cast<float>(States[D3DRS_LOCALVIEWER]),static_cast<float>(States[D3DRS_COLORVERTEX]),static_cast<float>(States[D3DRS_RANGEFOGENABLE])};
    v.Sources={static_cast<int>(States[D3DRS_DIFFUSEMATERIALSOURCE]),static_cast<int>(States[D3DRS_AMBIENTMATERIALSOURCE]),static_cast<int>(States[D3DRS_SPECULARMATERIALSOURCE]),static_cast<int>(States[D3DRS_EMISSIVEMATERIALSOURCE])};
    for(unsigned i=0;i<8;++i){
        v.TextureTransforms[i]=MatrixAt(D3DTS_TEXTURE0+i);v.Coordinates[i]={static_cast<int>(Stages[i][D3DTSS_TEXCOORDINDEX]),static_cast<int>(Stages[i][D3DTSS_TEXTURETRANSFORMFLAGS]),0,0};
        if(!LightEnabled[i])continue;
        auto& source=Lights[i];auto& light=v.Lights[i];
        light.Position=Transform(Vector(source.Position,1),v.View);light.Direction=Transform(Vector(source.Direction),v.View);
        light.Diffuse=Color(source.Diffuse);light.Ambient=Color(source.Ambient);light.Specular=Color(source.Specular);
        light.Parameters={source.Range,source.Falloff,source.Attenuation0,source.Attenuation1};
        light.Settings={source.Attenuation2,std::cos(source.Theta*0.5f),std::cos(source.Phi*0.5f),static_cast<float>(source.Type)};
    }
    FragmentState f{};f.Factor=Color(States[D3DRS_TEXTUREFACTOR]);f.FogColor=Color(States[D3DRS_FOGCOLOR]);
    auto number=[](DWORD value){return std::bit_cast<float>(value);};
    f.Fog={number(States[D3DRS_FOGSTART]),number(States[D3DRS_FOGEND]),number(States[D3DRS_FOGDENSITY]),static_cast<float>(States[D3DRS_FOGTABLEMODE]?States[D3DRS_FOGTABLEMODE]:States[D3DRS_FOGVERTEXMODE])};
    v.Fog=f.Fog;v.FogSettings={static_cast<int>(States[D3DRS_FOGTABLEMODE]),static_cast<int>(States[D3DRS_FOGVERTEXMODE]),0,0};
    if(States[D3DRS_FOGTABLEMODE]){
        bool affine=v.Projection[3]==0 && v.Projection[7]==0 && v.Projection[11]==0 && v.Projection[15]==1;
        f.Alpha.y=affine?1.0f:0.0f;
    }else f.Fog.w=-1;
    f.Flags={static_cast<int>(States[D3DRS_ALPHATESTENABLE]),static_cast<int>(States[D3DRS_ALPHAFUNC]),static_cast<int>(States[D3DRS_SPECULARENABLE]),static_cast<int>(States[D3DRS_FOGENABLE])};f.Alpha.x=static_cast<float>(States[D3DRS_ALPHAREF]&255);
    for(unsigned i=0;i<8;++i){auto& s=Stages[i];auto& out=f.Stages[i];
        out.Color={static_cast<int>(s[D3DTSS_COLOROP]),static_cast<int>(s[D3DTSS_COLORARG1]),static_cast<int>(s[D3DTSS_COLORARG2]),static_cast<int>(s[D3DTSS_COLORARG0])};
        out.Alpha={static_cast<int>(s[D3DTSS_ALPHAOP]),static_cast<int>(s[D3DTSS_ALPHAARG1]),static_cast<int>(s[D3DTSS_ALPHAARG2]),static_cast<int>(s[D3DTSS_ALPHAARG0])};
        if(!Textures[i]){
            bool first=s[D3DTSS_COLOROP]!=D3DTOP_SELECTARG2 && (s[D3DTSS_COLORARG1]&15)==D3DTA_TEXTURE;
            bool second=s[D3DTSS_COLOROP]!=D3DTOP_SELECTARG1 && (s[D3DTSS_COLORARG2]&15)==D3DTA_TEXTURE;
            if(first || second)out.Color.x=D3DTOP_DISABLE;
        }
        out.State={static_cast<int>(s[D3DTSS_RESULTARG]),static_cast<int>(s[D3DTSS_TEXTURETRANSFORMFLAGS]),static_cast<int>(s[D3DTSS_ADDRESSU]),static_cast<int>(s[D3DTSS_ADDRESSV])};
        out.Sampling={number(s[D3DTSS_MIPMAPLODBIAS]),0,number(s[D3DTSS_BUMPENVLSCALE]),number(s[D3DTSS_BUMPENVLOFFSET])};
        out.Bump={number(s[D3DTSS_BUMPENVMAT00]),number(s[D3DTSS_BUMPENVMAT10]),number(s[D3DTSS_BUMPENVMAT01]),number(s[D3DTSS_BUMPENVMAT11])};out.Border=Color(s[D3DTSS_BORDERCOLOR]);
    }
    SDL_PushGPUVertexUniformData(Ctx->Commands,0,&v,sizeof(v));SDL_PushGPUFragmentUniformData(Ctx->Commands,0,&f,sizeof(f));return true;
}
HRESULT Device::Draw(D3DPRIMITIVETYPE type,BufferData& vertices,unsigned stride,BufferData* indices,unsigned start,unsigned count,unsigned base)
{
    unsigned elements=ElementCount(type,count);
    if(!elements || !stride || !Target || !PrepareVertices(vertices,FVF,stride))return D3DERR_INVALIDCALL;
    unsigned indexBytes=indices && indices->Format==D3DFMT_INDEX32?4:2;
    if(indices){if(static_cast<size_t>(start+elements)*indexBytes>indices->Bytes.size() || !PrepareIndices(*indices))return D3DERR_INVALIDCALL;}
    else if(static_cast<size_t>(start+elements)*stride>vertices.Bytes.size())return D3DERR_INVALIDCALL;
    for(auto* texture:Textures)if(texture && !Ctx->Upload(*texture->Data))return Failure("Upload draw texture");
    if(!Ctx->Upload(*White))return Failure("Upload white texture");
    std::unique_ptr<BufferData> fan;
    if(type==D3DPT_TRIANGLEFAN){
        fan=std::make_unique<BufferData>(Ctx,count*3*4,0,D3DPOOL_DEFAULT);fan->Format=D3DFMT_INDEX32;
        auto index=[&](unsigned i){unsigned value=start+i;if(indices){value=0;std::memcpy(&value,indices->Bytes.data()+(start+i)*indexBytes,indexBytes);}return value;};
        for(unsigned i=0;i<count;++i){std::array<unsigned,3> triangle{index(0),index(i+1),index(i+2)};std::memcpy(fan->Bytes.data()+i*12,triangle.data(),12);}
        if(!PrepareIndices(*fan))return Failure("Upload triangle fan");indices=fan.get();indexBytes=4;start=0;elements=count*3;
    }
    auto* pipeline=Pipeline(type);if(!pipeline)return Failure("Create drawing pipeline");
    std::array<SDL_GPUTextureSamplerBinding,8> bindings{};
    for(unsigned i=0;i<8;++i){bindings[i].texture=Textures[i]?Textures[i]->Data->GPU:White->GPU;bindings[i].sampler=Sampler(i);if(!bindings[i].sampler)return Failure("Create texture sampler");}
    if(!Pass())return Failure("Begin draw pass");
    SDL_BindGPUGraphicsPipeline(Ctx->Pass,pipeline);
    SDL_GPUViewport viewport{static_cast<float>(Viewport.X),static_cast<float>(Viewport.Y),static_cast<float>(Viewport.Width),static_cast<float>(Viewport.Height),Viewport.MinZ,Viewport.MaxZ};SDL_SetGPUViewport(Ctx->Pass,&viewport);
    SDL_Rect scissor{static_cast<int>(Viewport.X),static_cast<int>(Viewport.Y),static_cast<int>(Viewport.Width),static_cast<int>(Viewport.Height)};SDL_SetGPUScissor(Ctx->Pass,&scissor);
    SDL_SetGPUStencilReference(Ctx->Pass,static_cast<Uint8>(States[D3DRS_STENCILREF]));
    SDL_GPUBufferBinding vertex{vertices.GPU,0};SDL_BindGPUVertexBuffers(Ctx->Pass,0,&vertex,1);
    SDL_BindGPUFragmentSamplers(Ctx->Pass,0,bindings.data(),8);Uniforms();
    if(indices){SDL_GPUBufferBinding index{indices->GPU,0};SDL_BindGPUIndexBuffer(Ctx->Pass,&index,indexBytes==4?SDL_GPU_INDEXELEMENTSIZE_32BIT:SDL_GPU_INDEXELEMENTSIZE_16BIT);SDL_DrawGPUIndexedPrimitives(Ctx->Pass,elements,1,start,static_cast<Sint32>(base),0);}
    else SDL_DrawGPUPrimitives(Ctx->Pass,elements,1,start,0);
    return D3D_OK;
}
HRESULT Device::DrawPrimitiveUP(D3DPRIMITIVETYPE type,UINT count,const void* data,UINT stride)
{
    if(!data || !stride)return D3DERR_INVALIDCALL;
    BufferData vertices(Ctx,ElementCount(type,count)*stride,0,D3DPOOL_SYSTEMMEM);std::memcpy(vertices.Bytes.data(),data,vertices.Bytes.size());
    HRESULT result=Draw(type,vertices,stride,nullptr,0,count,0);Bind<VertexBuffer>(Vertices,nullptr);Stride=0;return result;
}
HRESULT Device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type,UINT minimum,UINT vertexCount,UINT count,const void* indexData,D3DFORMAT format,const void* data,UINT stride)
{
    if(!data || !indexData || !stride || (format!=D3DFMT_INDEX16 && format!=D3DFMT_INDEX32))return D3DERR_INVALIDCALL;
    BufferData vertices(Ctx,(minimum+vertexCount)*stride,0,D3DPOOL_SYSTEMMEM);std::memcpy(vertices.Bytes.data(),data,vertices.Bytes.size());
    BufferData indices(Ctx,ElementCount(type,count)*(format==D3DFMT_INDEX32?4:2),0,D3DPOOL_SYSTEMMEM);indices.Format=format;std::memcpy(indices.Bytes.data(),indexData,indices.Bytes.size());
    HRESULT result=Draw(type,vertices,stride,&indices,0,count,0);Bind<VertexBuffer>(Vertices,nullptr);Bind<IndexBuffer>(Indices,nullptr);Stride=BaseVertex=0;return result;
}
bool Device::UploadGamma()
{
    if(!GammaDirty && GammaTexture)return true;
    Ctx->EndPass();if(!Ctx->Command())return false;
    if(!GammaTexture){SDL_GPUTextureCreateInfo info{};info.type=SDL_GPU_TEXTURETYPE_2D;info.format=SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;info.usage=SDL_GPU_TEXTUREUSAGE_SAMPLER;info.width=256;info.height=info.layer_count_or_depth=info.num_levels=1;info.sample_count=SDL_GPU_SAMPLECOUNT_1;GammaTexture=SDL_CreateGPUTexture(Ctx->GPU,&info);if(!GammaTexture)return false;}
    std::array<Float4,256> pixels{};for(unsigned i=0;i<256;++i)pixels[i]={Gamma.red[i]/65535.0f,Gamma.green[i]/65535.0f,Gamma.blue[i]/65535.0f,1};
    SDL_GPUTransferBufferCreateInfo info{};info.usage=SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;info.size=sizeof(pixels);
    auto* transfer=SDL_CreateGPUTransferBuffer(Ctx->GPU,&info);if(!transfer)return false;
    void* map=SDL_MapGPUTransferBuffer(Ctx->GPU,transfer,false);if(!map){SDL_ReleaseGPUTransferBuffer(Ctx->GPU,transfer);return false;}
    std::memcpy(map,pixels.data(),sizeof(pixels));SDL_UnmapGPUTransferBuffer(Ctx->GPU,transfer);
    auto* pass=SDL_BeginGPUCopyPass(Ctx->Commands);SDL_GPUTextureTransferInfo source{};source.transfer_buffer=transfer;source.pixels_per_row=256;source.rows_per_layer=1;
    SDL_GPUTextureRegion region{};region.texture=GammaTexture;region.w=256;region.h=region.d=1;
    SDL_UploadToGPUTexture(pass,&source,&region,true);SDL_EndGPUCopyPass(pass);SDL_ReleaseGPUTransferBuffer(Ctx->GPU,transfer);GammaDirty=false;return true;
}
HRESULT Device::ResourceManagerDiscardBytes(DWORD bytes)
{
    if(!Ctx->Flush(true))return Failure("Evict managed resources");
    std::uint64_t released=0;
    for(auto it=Ctx->ManagedImages.begin();it!=Ctx->ManagedImages.end();){
        auto image=it->lock();if(!image){it=Ctx->ManagedImages.erase(it);continue;}++it;
        if(!image->GPU)continue;
        for(unsigned i=0;i<image->Levels.size();++i){auto& level=image->Levels[i];if(level.GPUWritten && !Ctx->ReadImage(*image,i))return Failure("Read managed resource");level.Dirty=true;released+=level.Bytes.size();}
        SDL_ReleaseGPUTexture(Ctx->GPU,image->GPU);image->GPU=nullptr;
        if(bytes && released>=bytes)break;
    }
    return D3D_OK;
}
HRESULT Device::Present(const RECT* source,const RECT* destination,HWND overrideWindow,const RGNDATA* dirty)
{
    if(source || destination || overrideWindow || dirty)return Unsupported("Present regions");
    Ctx->EndPass();if(!Ctx->Upload(*Backbuffer->Data) || !UploadGamma() || !Ctx->Command())return Failure("Prepare presentation");
    SDL_GPUTexture* swapchain=nullptr;Uint32 width=0,height=0;
    if(!SDL_WaitAndAcquireGPUSwapchainTexture(Ctx->Commands,Ctx->Window,&swapchain,&width,&height))return Failure("Acquire swapchain");
    if(!swapchain)return Ctx->Flush(false)?D3D_OK:Failure("Submit minimized frame");
    if(!PresentPipeline){
        SDL_GPUGraphicsPipelineCreateInfo info{};info.vertex_shader=Ctx->PresentVS;info.fragment_shader=Ctx->PresentPS;
        info.primitive_type=SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;info.rasterizer_state.fill_mode=SDL_GPU_FILLMODE_FILL;info.rasterizer_state.cull_mode=SDL_GPU_CULLMODE_NONE;
        info.multisample_state.sample_count=SDL_GPU_SAMPLECOUNT_1;
        SDL_GPUColorTargetDescription color{};color.format=SDL_GetGPUSwapchainTextureFormat(Ctx->GPU,Ctx->Window);info.target_info.color_target_descriptions=&color;info.target_info.num_color_targets=1;
        PresentPipeline=SDL_CreateGPUGraphicsPipeline(Ctx->GPU,&info);if(!PresentPipeline)return Failure("Create presentation pipeline");
    }
    if(!PresentSampler){SDL_GPUSamplerCreateInfo info{};info.min_filter=info.mag_filter=SDL_GPU_FILTER_LINEAR;info.mipmap_mode=SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;info.address_mode_u=info.address_mode_v=info.address_mode_w=SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;info.max_anisotropy=1;PresentSampler=SDL_CreateGPUSampler(Ctx->GPU,&info);if(!PresentSampler)return Failure("Create presentation sampler");}
    SDL_GPUColorTargetInfo target{};target.texture=swapchain;target.load_op=SDL_GPU_LOADOP_DONT_CARE;target.store_op=SDL_GPU_STOREOP_STORE;
    Ctx->Pass=SDL_BeginGPURenderPass(Ctx->Commands,&target,1,nullptr);if(!Ctx->Pass)return Failure("Begin presentation pass");
    SDL_BindGPUGraphicsPipeline(Ctx->Pass,PresentPipeline);
    SDL_GPUViewport viewport{0,0,static_cast<float>(width),static_cast<float>(height),0,1};SDL_SetGPUViewport(Ctx->Pass,&viewport);
    SDL_GPUTextureSamplerBinding bindings[]={{Backbuffer->Data->GPU,PresentSampler},{GammaTexture,PresentSampler}};SDL_BindGPUFragmentSamplers(Ctx->Pass,0,bindings,2);
    SDL_DrawGPUPrimitives(Ctx->Pass,3,1,0,0);return Ctx->Flush(false)?D3D_OK:Failure("Submit frame");
}
struct SwapChain final : Object<IDirect3DSwapChain8> {
    std::shared_ptr<Context> Ctx;
    Surface* Backbuffer;
    SwapChain(std::shared_ptr<Context> ctx,Surface* surface):Ctx(std::move(ctx)),Backbuffer(surface) {Backbuffer->AddRef();}
    ~SwapChain() override {Backbuffer->Release();}
    HRESULT STDMETHODCALLTYPE Present(const RECT*,const RECT*,HWND,const RGNDATA*) override {return Unsupported("Additional swapchain presentation");}
    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT index,D3DBACKBUFFER_TYPE type,IDirect3DSurface8** output) override {if(index || type!=D3DBACKBUFFER_TYPE_MONO)return D3DERR_INVALIDCALL;return Return<IDirect3DSurface8>(Backbuffer,output);}
};
HRESULT Device::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* parameters,IDirect3DSwapChain8** output)
{
    if(!parameters || !output)return D3DERR_INVALIDCALL;
    IDirect3DSurface8* surface=nullptr;HRESULT result=CreateRenderTarget(parameters->BackBufferWidth,parameters->BackBufferHeight,parameters->BackBufferFormat,parameters->MultiSampleType,TRUE,&surface);
    if(FAILED(result))return result;
    *output=new SwapChain(Ctx,static_cast<Surface*>(surface));surface->Release();return D3D_OK;
}
struct Factory final : Object<IDirect3D8> {
    std::shared_ptr<Context> Ctx;
    std::vector<D3DDISPLAYMODE> Modes;
    explicit Factory(std::shared_ptr<Context> context):Ctx(std::move(context))
    {
        SDL_DisplayID display=SDL_GetPrimaryDisplay();int count=0;
        auto** modes=SDL_GetFullscreenDisplayModes(display,&count);
        if(modes){for(int i=0;i<count;++i)for(auto format:{D3DFMT_X8R8G8B8,D3DFMT_R5G6B5}){
            D3DDISPLAYMODE mode{static_cast<UINT>(modes[i]->w),static_cast<UINT>(modes[i]->h),static_cast<UINT>(std::round(modes[i]->refresh_rate)),format};
            if(std::none_of(Modes.begin(),Modes.end(),[&](auto& m){return std::memcmp(&m,&mode,sizeof(mode))==0;}))Modes.push_back(mode);
        }SDL_free(modes);}
    }
    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void*) override {return Unsupported("RegisterSoftwareDevice");}
    UINT STDMETHODCALLTYPE GetAdapterCount() override {return 1;}
    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT adapter,DWORD,D3DADAPTER_IDENTIFIER8* identifier) override
    {
        if(adapter || !identifier)return D3DERR_INVALIDCALL;*identifier={};
        SDL_strlcpy(identifier->Driver,"SDL GPU",sizeof(identifier->Driver));SDL_snprintf(identifier->Description,sizeof(identifier->Description),"SDL GPU (%s)",SDL_GetGPUDeviceDriver(Ctx->GPU));
        identifier->DeviceIdentifier={0xe814cf36,0xb35a,0x4f91,{0x99,0x3c,0x3d,0x95,0xf2,0xa4,0x07,0xc6}};return D3D_OK;
    }
    UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT adapter) override {return adapter?0:static_cast<UINT>(Modes.size());}
    HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT adapter,UINT index,D3DDISPLAYMODE* output) override {if(adapter || !output || index>=Modes.size())return D3DERR_INVALIDCALL;*output=Modes[index];return D3D_OK;}
    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT adapter,D3DDISPLAYMODE* output) override
    {
        if(adapter || !output)return D3DERR_INVALIDCALL;
        auto* mode=SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());if(!mode)return Failure("Get desktop mode");
        *output={static_cast<UINT>(mode->w),static_cast<UINT>(mode->h),static_cast<UINT>(std::round(mode->refresh_rate)),D3DFMT_X8R8G8B8};return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT adapter,D3DDEVTYPE type,D3DFORMAT,D3DFORMAT buffer,BOOL) override {if(adapter || type!=D3DDEVTYPE_HAL)return D3DERR_NOTAVAILABLE;return CheckDeviceFormat(0,type,D3DFMT_X8R8G8B8,D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,buffer);}
    HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT adapter,D3DDEVTYPE type,D3DFORMAT,DWORD usage,D3DRESOURCETYPE resource,D3DFORMAT format) override
    {
        if(adapter || type!=D3DDEVTYPE_HAL || (resource!=D3DRTYPE_TEXTURE && resource!=D3DRTYPE_SURFACE) || (!BytesPerPixel(format) && !BlockBytes(format)))return D3DERR_NOTAVAILABLE;
        bool depth=(usage&D3DUSAGE_DEPTHSTENCIL)!=0;
        if(depth!=DepthFormat(format))return D3DERR_NOTAVAILABLE;
        SDL_GPUTextureUsageFlags flags=depth?SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET:SDL_GPU_TEXTUREUSAGE_SAMPLER;
        if(usage&D3DUSAGE_RENDERTARGET){if(format!=D3DFMT_A8R8G8B8 && format!=D3DFMT_X8R8G8B8 && format!=D3DFMT_R5G6B5 && format!=D3DFMT_A1R5G5B5 && format!=D3DFMT_X1R5G5B5 && format!=D3DFMT_A4R4G4B4)return D3DERR_NOTAVAILABLE;flags|=SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;}
        auto actual=GPUFormat(format);if(actual==SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT && !SDL_GPUTextureSupportsFormat(Ctx->GPU,actual,SDL_GPU_TEXTURETYPE_2D,flags))actual=SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
        return SDL_GPUTextureSupportsFormat(Ctx->GPU,actual,SDL_GPU_TEXTURETYPE_2D,flags)?D3D_OK:D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT adapter,D3DDEVTYPE type,D3DFORMAT,BOOL,D3DMULTISAMPLE_TYPE samples) override {return !adapter && type==D3DDEVTYPE_HAL && samples==D3DMULTISAMPLE_NONE?D3D_OK:D3DERR_NOTAVAILABLE;}
    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT adapter,D3DDEVTYPE type,D3DFORMAT display,D3DFORMAT target,D3DFORMAT depth) override {HRESULT result=CheckDeviceFormat(adapter,type,display,D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,target);return FAILED(result)?result:CheckDeviceFormat(adapter,type,display,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,depth);}
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT adapter,D3DDEVTYPE type,D3DCAPS8* caps) override
    {
        if(adapter || type!=D3DDEVTYPE_HAL || !caps)return D3DERR_INVALIDCALL;*caps={};caps->DeviceType=type;
        caps->Caps2=D3DCAPS2_FULLSCREENGAMMA|D3DCAPS2_CANRENDERWINDOWED|D3DCAPS2_DYNAMICTEXTURES;
        caps->PresentationIntervals=D3DPRESENT_INTERVAL_ONE|D3DPRESENT_INTERVAL_IMMEDIATE;
        caps->DevCaps=D3DDEVCAPS_HWTRANSFORMANDLIGHT|D3DDEVCAPS_HWRASTERIZATION|D3DDEVCAPS_DRAWPRIMITIVES2EX|D3DDEVCAPS_TEXTURESYSTEMMEMORY|D3DDEVCAPS_TEXTUREVIDEOMEMORY|D3DDEVCAPS_TLVERTEXSYSTEMMEMORY|D3DDEVCAPS_TLVERTEXVIDEOMEMORY;
        caps->PrimitiveMiscCaps=D3DPMISCCAPS_CULLNONE|D3DPMISCCAPS_CULLCW|D3DPMISCCAPS_CULLCCW|D3DPMISCCAPS_COLORWRITEENABLE|D3DPMISCCAPS_BLENDOP;
        caps->RasterCaps=D3DPRASTERCAPS_ZTEST|D3DPRASTERCAPS_FOGVERTEX|D3DPRASTERCAPS_FOGTABLE|D3DPRASTERCAPS_FOGRANGE|D3DPRASTERCAPS_MIPMAPLODBIAS|D3DPRASTERCAPS_ZBIAS|D3DPRASTERCAPS_ANISOTROPY;
        caps->ZCmpCaps=caps->AlphaCmpCaps=0xff;caps->SrcBlendCaps=caps->DestBlendCaps=0x1fff;
        caps->ShadeCaps=D3DPSHADECAPS_COLORGOURAUDRGB|D3DPSHADECAPS_SPECULARGOURAUDRGB|D3DPSHADECAPS_ALPHAGOURAUDBLEND|D3DPSHADECAPS_FOGGOURAUD;
        caps->TextureCaps=D3DPTEXTURECAPS_PERSPECTIVE|D3DPTEXTURECAPS_ALPHA|D3DPTEXTURECAPS_ALPHAPALETTE|D3DPTEXTURECAPS_PROJECTED|D3DPTEXTURECAPS_MIPMAP;
        caps->TextureFilterCaps=D3DPTFILTERCAPS_MINFPOINT|D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MINFANISOTROPIC|D3DPTFILTERCAPS_MAGFPOINT|D3DPTFILTERCAPS_MAGFLINEAR|D3DPTFILTERCAPS_MAGFANISOTROPIC|D3DPTFILTERCAPS_MIPFPOINT|D3DPTFILTERCAPS_MIPFLINEAR;
        caps->TextureAddressCaps=D3DPTADDRESSCAPS_WRAP|D3DPTADDRESSCAPS_MIRROR|D3DPTADDRESSCAPS_CLAMP|D3DPTADDRESSCAPS_BORDER|D3DPTADDRESSCAPS_MIRRORONCE|D3DPTADDRESSCAPS_INDEPENDENTUV;
        caps->MaxTextureWidth=caps->MaxTextureHeight=caps->MaxTextureAspectRatio=4096;caps->MaxTextureRepeat=8192;caps->MaxAnisotropy=16;caps->MaxVertexW=1e10f;
        caps->StencilCaps=0xff;caps->FVFCaps=8;caps->TextureOpCaps=0x3ffffff;caps->MaxTextureBlendStages=caps->MaxSimultaneousTextures=8;
        caps->VertexProcessingCaps=D3DVTXPCAPS_TEXGEN|D3DVTXPCAPS_MATERIALSOURCE7|D3DVTXPCAPS_DIRECTIONALLIGHTS|D3DVTXPCAPS_POSITIONALLIGHTS|D3DVTXPCAPS_LOCALVIEWER;
        caps->MaxActiveLights=8;caps->MaxPointSize=1;caps->MaxPrimitiveCount=0x555555;caps->MaxVertexIndex=0xffffff;caps->MaxStreams=1;caps->MaxStreamStride=256;
        return D3D_OK;
    }
    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT) override {return nullptr;}
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT adapter,D3DDEVTYPE type,HWND focus,DWORD behavior,D3DPRESENT_PARAMETERS* parameters,IDirect3DDevice8** output) override
    {
        if(adapter || type!=D3DDEVTYPE_HAL || !parameters || !output || Ctx->Owner)return D3DERR_INVALIDCALL;*output=nullptr;
        Ctx->Window=Platform::GetWindow();if(!Ctx->Window)return Failure("Get SDL window");
        if(!Ctx->Claimed){if(!SDL_ClaimWindowForGPUDevice(Ctx->GPU,Ctx->Window))return Failure("Claim SDL window");Ctx->Claimed=true;}
        D3DDEVICE_CREATION_PARAMETERS creation{adapter,type,focus,behavior};auto* device=new Device(Ctx,this,creation);
        HRESULT result=device->Reset(parameters);if(FAILED(result)){device->Release();return result;}
        *output=device;return D3D_OK;
    }
};
}

IDirect3D8* Platform::CreateD3D8Renderer()
{
    auto context=std::make_shared<Context>();
    if(!context->Initialize()){Failure("Initialize SDL GPU");return nullptr;}
    return new Factory(std::move(context));
}
