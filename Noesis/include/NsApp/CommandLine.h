#ifndef __APP_COMMANDLINE_H__
#define __APP_COMMANDLINE_H__

#include <NsCore/Noesis.h>
#include <NsApp/IrrNoesis.h>

namespace NoesisApp
{

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Wrapper class for command line arguments
////////////////////////////////////////////////////////////////////////////////////////////////////
class NS_IRR_NOESIS_API CommandLine
{
public:
    CommandLine();
    CommandLine(uint32_t argc, char const* const* argv);

    uint32_t GetNumArgs() const;
    char const* const* GetArgs() const;
    const char* GetArg(uint32_t index) const;

    const char* FindOption(const char* option, const char* default_) const;
    bool HasOption(const char* option) const;

private:
    uint32_t mNumArgs;
    char const* const* mArgs;
};

}

#endif
