#include <UIKit/UIKit.h>
#include "../EnvExt.hpp"

namespace UI { namespace App {

Language EnvExt::detectLanguage(void) const {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];
    
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    
    Language ret = LANGUAGE_EN;
    if ([languageCode isEqualToString:@"zh"]) {
        ret = LANGUAGE_CN;
    }
    else if ([languageCode isEqualToString:@"en"]) {
        ret = LANGUAGE_EN;
    }

    return ret;
}

}}

