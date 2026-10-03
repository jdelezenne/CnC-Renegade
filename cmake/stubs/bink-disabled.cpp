// Temporary movie facade for builds without the proprietary Bink SDK.
#include "binkmovie.h"

void BINKMovie::Play(const char *, const char *, FontCharsClass *) {}
void BINKMovie::Stop() {}
void BINKMovie::Update() {}
void BINKMovie::Render() {}
void BINKMovie::Init() {}
void BINKMovie::Shutdown() {}
bool BINKMovie::Is_Complete() { return true; }
