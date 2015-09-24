//
//  TurnableProgressTimer.h
//
//
//  Created by bibi-apple on 28/1/15.
//
//

#ifndef __TurnableRadialProgressTimer__
#define __TurnableRadialProgressTimer__

#include "cocos2d.h"

NS_CC_BEGIN

class TurnableRadialProgressTimer : public Node
{
public:
    CREATE_FUNC(TurnableRadialProgressTimer);
    
    static TurnableRadialProgressTimer *create(Sprite *sprite);
    
    inline float getPercentage() const {return _percentage; }
    
    inline Sprite* getSprite() const { return _sprite; }
    
    void setPercentage(float percentage);
    void setSprite(Sprite *sprite);
    
    void setStartAngle(float angle);
    
    float getStartAngle() {
        return _startAngle;
    }
    

    void setReverseProgress(bool reverse);
    
    inline bool isReverseDirection() { return _reverseDirection; };
    inline void setReverseDirection(bool value) { _reverseDirection = value; };
    
    void setMidpoint(const Vec2& point);

    Vec2 getMidpoint() const;
    
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual void setAnchorPoint(const Vec2& anchorPoint) override;
    virtual void setColor(const Color3B &color) override;
    virtual const Color3B& getColor() const override;
    virtual void setOpacity(GLubyte opacity) override;
    virtual GLubyte getOpacity() const override;
    
    
    
CC_CONSTRUCTOR_ACCESS:
    TurnableRadialProgressTimer();

    virtual ~TurnableRadialProgressTimer();
    
    virtual bool init();
    bool initWithSprite(Sprite* sp);
    
protected:
    void onDraw(const Mat4 &transform, uint32_t flags);
    
    Tex2F textureCoordFromAlphaPoint(Vec2 alpha);
    Vec2 vertexFromAlphaPoint(Vec2 alpha);
    void updateRadial(void);
    virtual void updateColor(void) override;
    Vec2 boundaryTexCoord(char index);
    
    Vec2 _midpoint;

    float _percentage;
    Sprite *_sprite;
    int _vertexDataCount;
    V2F_C4B_T2F *_vertexData;
    
    float _startAngle;
    
    CustomCommand _customCommand;
    
    bool _reverseDirection;
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(TurnableRadialProgressTimer);
};

NS_CC_END

#endif /* defined(__TurnableRadialProgressTimer__) */
