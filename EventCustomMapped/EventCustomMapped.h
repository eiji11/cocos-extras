//
//  EventCustomMapped.h
//  WaterQuiz
//
//  Created by bibi-apple on 30/12/14.
//
//

#ifndef __EventCustomMapped__
#define __EventCustomMapped__

#include "cocos2d.h"

NS_CC_BEGIN

class EventCustomMapped : public EventCustom
{
public:
    EventCustomMapped(const std::string& eventName);
    
    void setInfo(ValueMap &info);
    ValueMap &getInfo();
    
protected:
    ValueMap _info;
    
};

NS_CC_END

#endif /* defined(__EventCustomMapped__) */
