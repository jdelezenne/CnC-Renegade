#pragma once

typedef std::int32_t time_t;

typedef 
enum Locale
    {	LOC_UNKNOWN	= 0,
	LOC_OTHER	= LOC_UNKNOWN + 1,
	LOC_USA	= LOC_OTHER + 1,
	LOC_CANADA	= LOC_USA + 1,
	LOC_UK	= LOC_CANADA + 1,
	LOC_GERMANY	= LOC_UK + 1,
	LOC_FRANCE	= LOC_GERMANY + 1,
	LOC_SPAIN	= LOC_FRANCE + 1,
	LOC_NETHERLANDS	= LOC_SPAIN + 1,
	LOC_BELGIUM	= LOC_NETHERLANDS + 1,
	LOC_AUSTRIA	= LOC_BELGIUM + 1,
	LOC_SWITZERLAND	= LOC_AUSTRIA + 1,
	LOC_ITALY	= LOC_SWITZERLAND + 1,
	LOC_DENMARK	= LOC_ITALY + 1,
	LOC_SWEDEN	= LOC_DENMARK + 1,
	LOC_NORWAY	= LOC_SWEDEN + 1,
	LOC_FINLAND	= LOC_NORWAY + 1,
	LOC_ISRAEL	= LOC_FINLAND + 1,
	LOC_SOUTH_AFRICA	= LOC_ISRAEL + 1,
	LOC_JAPAN	= LOC_SOUTH_AFRICA + 1,
	LOC_SOUTH_KOREA	= LOC_JAPAN + 1,
	LOC_CHINA	= LOC_SOUTH_KOREA + 1,
	LOC_SINGAPORE	= LOC_CHINA + 1,
	LOC_TAIWAN	= LOC_SINGAPORE + 1,
	LOC_MALAYSIA	= LOC_TAIWAN + 1,
	LOC_AUSTRALIA	= LOC_MALAYSIA + 1,
	LOC_NEW_ZEALAND	= LOC_AUSTRALIA + 1,
	LOC_BRAZIL	= LOC_NEW_ZEALAND + 1,
	LOC_THAILAND	= LOC_BRAZIL + 1,
	LOC_ARGENTINA	= LOC_THAILAND + 1,
	LOC_PHILIPPINES	= LOC_ARGENTINA + 1,
	LOC_GREECE	= LOC_PHILIPPINES + 1,
	LOC_IRELAND	= LOC_GREECE + 1,
	LOC_POLAND	= LOC_IRELAND + 1,
	LOC_PORTUGAL	= LOC_POLAND + 1,
	LOC_MEXICO	= LOC_PORTUGAL + 1,
	LOC_RUSSIA	= LOC_MEXICO + 1,
	LOC_TURKEY	= LOC_RUSSIA + 1
    }	Locale;

struct  Highscore
    {
    unsigned int sku;
    unsigned int wins;
    unsigned int losses;
    unsigned int points;
    unsigned int rank;
    unsigned int accomplishments;
    struct Highscore  *next;
    unsigned char login_name[ 40 ];
    };
struct  Ladder
    {
    unsigned int sku;
    unsigned int team_no;
    unsigned int wins;
    unsigned int losses;
    unsigned int points;
    unsigned int kills;
    unsigned int rank;
    unsigned int rung;
    unsigned int disconnects;
    unsigned int team_rung;
    unsigned int provisional;
    unsigned int last_game_date;
    unsigned int win_streak;
    unsigned int reserved1;
    unsigned int reserved2;
    struct Ladder  *next;
    unsigned char login_name[ 40 ];
    Locale locale;
    };
typedef int GroupID;

struct  Server
    {
    int gametype;
    int chattype;
    int timezone;
    float longitude;
    float lattitude;
    struct Server  *next;
    unsigned char name[ 71 ];
    unsigned char connlabel[ 5 ];
    unsigned char conndata[ 128 ];
    unsigned char login[ 10 ];
    unsigned char password[ 10 ];
    };
struct  Channel
    {
    int type;
    unsigned int minUsers;
    unsigned int maxUsers;
    unsigned int currentUsers;
    unsigned int official;
    unsigned int tournament;
    unsigned int ingame;
    unsigned int flags;
    std::uint32_t reserved;
    std::uint32_t ipaddr;
    int latency;
    int hidden;
    struct Channel  *next;
    unsigned char name[ 17 ];
    unsigned char topic[ 81 ];
    unsigned char location[ 65 ];
    unsigned char key[ 9 ];
    unsigned char exInfo[ 41 ];
    };
struct  User
    {
    unsigned int flags;
    GroupID group;
    std::uint32_t reserved;
    std::uint32_t reserved2;
    std::uint32_t reserved3;
    std::uint32_t squadID;
    std::uint32_t ipaddr;
    std::uint32_t squad_icon;
    struct User  *next;
    unsigned char name[ 10 ];
    unsigned char squadname[ 41 ];
    unsigned char squadabbrev[ 10 ];
    Locale locale;
    int team;
    };
struct  Group
    {
    GroupID ident;
    int type;
    unsigned int members;
    struct Group  *next;
    unsigned char name[ 65 ];
    };
struct  Squad
    {
    std::uint32_t id;
    int sku;
    int members;
    int color1;
    int color2;
    int color3;
    int icon1;
    int icon2;
    int icon3;
    struct Squad  *next;
    int rank;
    int team;
    int status;
    unsigned char email[ 81 ];
    unsigned char icq[ 17 ];
    unsigned char motto[ 81 ];
    unsigned char url[ 129 ];
    unsigned char name[ 41 ];
    unsigned char abbreviation[ 41 ];
    };
struct  Update
    {
    std::uint32_t SKU;
    std::uint32_t version;
    int required;
    struct Update  *next;
    unsigned char server[ 65 ];
    unsigned char patchpath[ 256 ];
    unsigned char patchfile[ 33 ];
    unsigned char login[ 33 ];
    unsigned char password[ 65 ];
    unsigned char localpath[ 256 ];
    };
typedef struct Server Server;

typedef struct Channel Channel;

typedef struct User User;

typedef struct Group Group;

typedef struct Update Update;

typedef struct Ladder Ladder;

typedef struct Highscore Highscore;

typedef struct Squad Squad;

struct IRTPatcher : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE ApplyPatch( 
             LPCSTR destpath,
             LPCSTR filename) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE PumpMessages( void) = 0;
};

struct IRTPatcherEvent : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE OnProgress( 
             LPCSTR filename,
             int progress) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnTermination( 
             BOOL success) = 0;
};

struct IChat : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE PumpMessages( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestServerList( 
             unsigned long SKU,
             unsigned long current_version,
             LPCSTR loginname,
             LPCSTR password,
             int timeout) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestConnection( 
             Server  *server,
             int timeout,
            int domangle) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelList( 
             int channelType,
             int autoping) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelCreate( 
             Channel  *channel) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelJoin( 
             Channel  *channel) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelLeave( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserList( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPublicMessage( 
             LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPrivateMessage( 
             User  *users,
             LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLogout( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPrivateGameOptions( 
             User  *users,
             LPCSTR options) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPublicGameOptions( 
             LPCSTR options) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPublicAction( 
             LPCSTR action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPrivateAction( 
             User  *users,
             LPCSTR action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestGameStart( 
             User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelTopic( 
             LPCSTR topic) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetVersion( 
             unsigned long  *version) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserKick( 
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserIP( 
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetGametypeInfo( 
            unsigned int gtype,
            int icon_size,
            unsigned char  * *bitmap,
            int  *bmp_bytes,
            LPCSTR  *name,
            LPCSTR  *URL) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestFind( 
            User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPage( 
            User  *user,
            LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetFindPage( 
            int findOn,
            int pageOn) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetSquelch( 
            User  *user,
            int squelch) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetSquelch( 
            User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetChannelFilter( 
            int channelType) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestGameEnd( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetLangFilter( 
            int onoff) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelBan( 
            LPCSTR name,
            int ban) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetGametypeList( 
            LPCSTR  *list) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetHelpURL( 
            LPCSTR  *url) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetProductSKU( 
            unsigned long SKU) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetNick( 
            int num,
            LPCSTR  *nick,
            LPCSTR  *pass) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetNick( 
            int num,
            LPCSTR nick,
            LPCSTR pass,
            int domangle) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetLobbyCount( 
            int  *count) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestRawMessage( 
            LPCSTR ircmsg) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetAttributeValue( 
            LPCSTR attrib,
            LPCSTR  *value) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetAttributeValue( 
            LPCSTR attrib,
            LPCSTR value) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetChannelExInfo( 
            LPCSTR info) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE StopAutoping( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSquadInfo( 
            unsigned long id) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSetTeam( 
            int team) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSetLocale( 
            Locale locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserLocale( 
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserTeam( 
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetNickLocale( 
            int nicknum,
            Locale  *locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetNickLocale( 
            int nicknum,
            Locale locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetLocaleString( 
            LPCSTR  *loc_string,
            Locale locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetLocaleCount( 
            int  *num) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetClientVersion( 
            unsigned long version) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetCodepageFilter( 
            int filter) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestBuddyList( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestBuddyAdd( 
            User  *newbuddy) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestBuddyDelete( 
            User  *buddy) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPublicUnicodeMessage( 
             const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPrivateUnicodeMessage( 
             User  *users,
             const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPublicUnicodeAction( 
             const unsigned short  *action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPrivateUnicodeAction( 
             User  *users,
             const unsigned short  *action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUnicodePage( 
            User  *user,
            const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSetPlayerCount( 
            unsigned int currentPlayers,
            unsigned int maxPlayers) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestServerTime( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestInsiderStatus( 
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSetLocalIP( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestSquadByName( 
            LPCSTR name) = 0;
};

struct IChatEvent : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE OnServerList( 
             HRESULT res,
             Server  *servers) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUpdateList( 
             HRESULT res,
             Update  *updates) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnServerError( 
             HRESULT res,
             LPCSTR ircmsg) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnConnection( 
             HRESULT res,
             LPCSTR motd) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnMessageOfTheDay( 
             HRESULT res,
             LPCSTR motd) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelList( 
             HRESULT res,
             Channel  *channels) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelCreate( 
             HRESULT res,
             Channel  *channel) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelJoin( 
             HRESULT res,
             Channel  *channel,
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelLeave( 
             HRESULT res,
             Channel  *channel,
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelTopic( 
             HRESULT res,
             Channel  *channel,
             LPCSTR topic) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPrivateAction( 
             HRESULT res,
             User  *user,
             LPCSTR action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPublicAction( 
             HRESULT res,
             Channel  *channel,
            User  *user,
             LPCSTR action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserList( 
             HRESULT res,
             Channel  *channel,
             User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPublicMessage( 
             HRESULT res,
             Channel  *channel,
             User  *user,
             LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPrivateMessage( 
             HRESULT res,
             User  *user,
             LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnSystemMessage( 
             HRESULT res,
             LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnNetStatus( 
             HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnLogout( 
             HRESULT status,
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPrivateGameOptions( 
             HRESULT res,
             User  *user,
             LPCSTR options) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPublicGameOptions( 
             HRESULT res,
             Channel  *channel,
             User  *user,
             LPCSTR options) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnGameStart( 
             HRESULT res,
             Channel  *channel,
             User  *users,
             int gameid) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserKick( 
             HRESULT res,
             Channel  *channel,
             User  *kicked,
             User  *kicker) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserIP( 
             HRESULT res,
             User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnFind( 
            HRESULT res,
            Channel  *chan) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPageSend( 
            HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPaged( 
            HRESULT res,
            User  *user,
            LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnServerBannedYou( 
            HRESULT res,
            time_t bannedTill) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserFlags( 
            HRESULT res,
            LPCSTR name,
            unsigned int flags,
            unsigned int mask) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelBan( 
            HRESULT res,
            LPCSTR name,
            int banned) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnSquadInfo( 
            HRESULT res,
            unsigned long id,
            Squad  *squad) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserLocale( 
            HRESULT res,
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserTeam( 
            HRESULT res,
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnSetLocale( 
            HRESULT res,
            Locale newlocale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnSetTeam( 
            HRESULT res,
            int newteam) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnBuddyList( 
            HRESULT res,
            User  *buddy_list) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnBuddyAdd( 
            HRESULT res,
            User  *buddy_added) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnBuddyDelete( 
            HRESULT res,
            User  *buddy_deleted) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPublicUnicodeMessage( 
             HRESULT res,
             Channel  *channel,
             User  *user,
             const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPrivateUnicodeMessage( 
             HRESULT res,
             User  *user,
             const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPrivateUnicodeAction( 
             HRESULT res,
             User  *user,
             const unsigned short  *action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPublicUnicodeAction( 
             HRESULT res,
             Channel  *channel,
            User  *user,
             const unsigned short  *action) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnPagedUnicode( 
            HRESULT res,
            User  *user,
            const unsigned short  *message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnServerTime( 
            HRESULT res,
            time_t stime) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnInsiderStatus( 
            HRESULT res,
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnSetLocalIP( 
            HRESULT res,
            LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelListBegin( 
             HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelListEntry( 
             HRESULT res,
             Channel  *channel) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelListEnd( 
             HRESULT res) = 0;
};

struct IDownload : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE DownloadFile( 
            LPCSTR server,
            LPCSTR login,
            LPCSTR password,
            LPCSTR file,
            LPCSTR localfile,
            LPCSTR regkey) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE PumpMessages( void) = 0;
};

struct IDownloadEvent : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE OnEnd( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnError( 
            int error) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnProgressUpdate( 
            int bytesread,
            int totalsize,
            int timetaken,
            int timeleft) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnQueryResume( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnStatusUpdate( 
            int status) = 0;
};

struct INetUtil : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE RequestGameresSend( 
            LPCSTR host,
            int port,
            unsigned char  *data,
            int length) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLadderSearch( 
            LPCSTR host,
            int port,
            LPCSTR key,
            unsigned long SKU,
            int team,
            int cond,
            int sort,
            int number,
            int leading) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLadderList( 
            LPCSTR host,
            int port,
            LPCSTR keys,
            unsigned long SKU,
            int team,
            int cond,
            int sort) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestPing( 
            LPCSTR host,
            int timeout,
            int  *handle) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE PumpMessages( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetAvgPing( 
            unsigned long ip,
            int  *avg) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestNewNick( 
            LPCSTR nick,
            LPCSTR pass,
            LPCSTR email,
            LPCSTR parentEmail,
            int newsletter,
            int shareinfo) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestAgeCheck( 
            int month,
            int day,
            int year,
            LPCSTR email) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestWDTState( 
            LPCSTR host,
            int port,
            unsigned char request) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLocaleLadderList( 
            LPCSTR host,
            int port,
            LPCSTR keys,
            unsigned long SKU,
            int team,
            int cond,
            int sort,
            Locale locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLocaleLadderSearch( 
            LPCSTR host,
            int port,
            LPCSTR key,
            unsigned long sku,
            int team,
            int cond,
            int sort,
            int number,
            int leading,
            Locale locale) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestHighscore( 
            LPCSTR host,
            int port,
            LPCSTR keys,
            unsigned long SKU) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE SetGameResMD5( 
            int flag) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLargeGameresSend( 
            LPCSTR host,
            int port,
            unsigned char  *data,
            unsigned long length) = 0;
};

struct INetUtilEvent : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE OnPing( 
            HRESULT res,
            int time,
            unsigned long ip,
            int handle) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnLadderList( 
            HRESULT res,
             Ladder  *list,
            int totalCount,
            long timeStamp,
            int keyRung) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnGameresSent( 
            HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnNewNick( 
            HRESULT res,
            LPCSTR message,
            LPCSTR nick,
            LPCSTR pass) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnAgeCheck( 
            HRESULT res,
            int years,
            int consent) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnWDTState( 
            HRESULT res,
            unsigned char  *state,
            int length) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnHighscore( 
            HRESULT res,
             Highscore  *list,
            int totalCount,
            long timeStamp,
            int keyRung) = 0;
};

typedef unsigned long GID;


enum GTYPE_
    {	SERVER	= 0,
	CHANNEL	= 1,
	CLIENT	= 2
    };
typedef enum GTYPE_ GTYPE;


enum CHAN_CTYPE_
    {	ALLEXIT	= 0,
	CREATOREXIT	= 1,
	CLOSEC	= 2
    };
typedef enum CHAN_CTYPE_ CHAN_CTYPE;



struct IChat2 : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE PumpMessages( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestConnection( 
            Server  *server,
            int timeout) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestMessage( 
            GID who,
            LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE GetTypeFromGID( 
            GID id,
            GTYPE  *type) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelList( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelJoin( 
            LPCSTR name) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelLeave( 
            Channel  *chan) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestUserList( 
            Channel  *chan) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestLogout( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestChannelCreate( 
            Channel  *chan) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE RequestRawCmd( 
            LPCSTR cmd) = 0;
};

struct IChat2Event : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE OnNetStatus( 
            HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnMessage( 
            HRESULT res,
            User  *user,
            LPCSTR message) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelList( 
            HRESULT res,
            Channel  *list) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelJoin( 
            HRESULT res,
            Channel  *chan,
            User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnLogin( 
            HRESULT res) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUserList( 
            HRESULT res,
            Channel  *chan,
            User  *users) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelLeave( 
            HRESULT res,
            Channel  *chan,
            User  *user) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnChannelCreate( 
            HRESULT res,
            Channel  *chan) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE OnUnknownLine( 
            HRESULT res,
            LPCSTR line) = 0;
};

struct IIGROptions : Platform::OnlineInterface {
public:
        virtual  HRESULT STDMETHODCALLTYPE Init( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE Is_Auto_Login_Allowed( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE Is_Storing_Nicks_Allowed( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE Is_Running_Reg_App_Allowed( void) = 0;
        
        virtual  HRESULT STDMETHODCALLTYPE Set_Options( 
            unsigned int options) = 0;
};

inline constexpr GUID IID_IRTPatcher = {0x925CDEDE,0x71B9,0x11D1,{0xB1,0xC5,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID IID_IRTPatcherEvent = {0x925CDEE3,0x71B9,0x11D1,{0xB1,0xC5,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID IID_IChat = {0x4DD3BAF4,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID IID_IChatEvent = {0x4DD3BAF6,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID IID_IDownload = {0x0BF5FCEB,0x9F03,0x11D1,{0x9D,0xC7,0x00,0x60,0x97,0xC5,0x43,0x21}};
inline constexpr GUID IID_IDownloadEvent = {0x6869E99D,0x9FB4,0x11D1,{0x9D,0xC8,0x00,0x60,0x97,0xC5,0x43,0x21}};
inline constexpr GUID IID_INetUtil = {0xB832B0AA,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};
inline constexpr GUID IID_INetUtilEvent = {0xB832B0AC,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};
inline constexpr GUID IID_IChat2 = {0x8B938190,0xEF3F,0x11D1,{0x98,0x08,0x00,0x60,0x97,0x06,0xFA,0x0C}};
inline constexpr GUID IID_IChat2Event = {0x8B938192,0xEF3F,0x11D1,{0x98,0x08,0x00,0x60,0x97,0x06,0xFA,0x0C}};
inline constexpr GUID IID_IIGROptions = {0x89DD1ECD,0x0DCA,0x49d8,{0x8E,0xF3,0x33,0x75,0xE6,0xD6,0xEE,0x9D}};
inline constexpr GUID LIBID_WOLAPILib = {0x925CDED1,0x71B9,0x11D1,{0xB1,0xC5,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID CLSID_RTPatcher = {0x925CDEDF,0x71B9,0x11D1,{0xB1,0xC5,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID CLSID_Chat = {0x4DD3BAF5,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};
inline constexpr GUID CLSID_Download = {0xBF6EA206,0x9E55,0x11D1,{0x9D,0xC6,0x00,0x60,0x97,0xC5,0x43,0x21}};
inline constexpr GUID CLSID_IGROptions = {0xABF6FC8F,0x1344,0x46de,{0x84,0xC9,0x83,0x71,0x11,0x8D,0xC3,0xFF}};
inline constexpr GUID CLSID_NetUtil = {0xB832B0AB,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};
inline constexpr GUID CLSID_Chat2 = {0x8B938191,0xEF3F,0x11D1,{0x98,0x08,0x00,0x60,0x97,0x06,0xFA,0x0C}};
