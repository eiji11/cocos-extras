#include "TurnableRadialProgressTimer.h"


NS_CC_BEGIN

#define kProgressTextureCoordsCount 4
//  kProgressTextureCoords holds points {0,1} {0,0} {1,0} {1,1} we can represent it as bits
const char kProgressTextureCoords = 0x4b;


TurnableRadialProgressTimer::TurnableRadialProgressTimer()
: Node()
,_midpoint(0,0)
,_percentage(0.0f)
,_sprite(nullptr)
,_vertexDataCount(0)
,_vertexData(nullptr)
,_reverseDirection(false)
,_startAngle(0)
{}

TurnableRadialProgressTimer* TurnableRadialProgressTimer::create(Sprite* sp)
{
    TurnableRadialProgressTimer *instance = new TurnableRadialProgressTimer();
    if (instance->initWithSprite(sp))
    {
        instance->autorelease();
    }
    else
    {
        delete instance;
        instance = nullptr;
    }
    
    return instance;
}

bool TurnableRadialProgressTimer::init()
{
    return initWithSprite(nullptr);
}

bool TurnableRadialProgressTimer::initWithSprite(Sprite* sp)
{
    setPercentage(0.0f);
    _vertexData = nullptr;
    _vertexDataCount = 0;
    
    setAnchorPoint(Vec2(0.5f,0.5f));
    _reverseDirection = false;
    setMidpoint(Vec2(0.5f, 0.5f));
    setSprite(sp);
    _startAngle = 0;
    
    // shader state
    setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR));
    return true;
}

TurnableRadialProgressTimer::~TurnableRadialProgressTimer(void)
{
    CC_SAFE_FREE(_vertexData);
    CC_SAFE_RELEASE(_sprite);
}

void TurnableRadialProgressTimer::setPercentage(float percentage)
{
    if (_percentage != percentage)
    {
        _percentage = clampf(percentage, 0, 100);
        updateRadial();
    }
}

void TurnableRadialProgressTimer::setSprite(Sprite *sprite)
{
    if (_sprite != sprite && sprite != nullptr)
    {
        CC_SAFE_RETAIN(sprite);
        CC_SAFE_RELEASE(_sprite);
        _sprite = sprite;
        setContentSize(_sprite->getContentSize());
        
        //    Every time we set a new sprite, we free the current vertex data
        if (_vertexData)
        {
            CC_SAFE_FREE(_vertexData);
            _vertexDataCount = 0;
        }
    }
}

void TurnableRadialProgressTimer::setStartAngle(float angle)
{
    _startAngle = angle;
    
    updateRadial();
}

void TurnableRadialProgressTimer::setReverseProgress(bool reverse)
{
    if( _reverseDirection != reverse ) {
        _reverseDirection = reverse;
        
        //    release all previous information
        CC_SAFE_FREE(_vertexData);
        _vertexDataCount = 0;
    }
}

// Interval

///
//    @returns the vertex position from the texture coordinate
///
Tex2F TurnableRadialProgressTimer::textureCoordFromAlphaPoint(Vec2 alpha)
{
    Tex2F ret(0.0f, 0.0f);
    if (!_sprite) {
        return ret;
    }
    V3F_C4B_T2F_Quad quad = _sprite->getQuad();
    Vec2 min = Vec2(quad.bl.texCoords.u,quad.bl.texCoords.v);
    Vec2 max = Vec2(quad.tr.texCoords.u,quad.tr.texCoords.v);
    //  Fix bug #1303 so that progress timer handles sprite frame texture rotation
    if (_sprite->isTextureRectRotated()) {
        CC_SWAP(alpha.x, alpha.y, float);
    }
    return Tex2F(min.x * (1.f - alpha.x) + max.x * alpha.x, min.y * (1.f - alpha.y) + max.y * alpha.y);
}

Vec2 TurnableRadialProgressTimer::vertexFromAlphaPoint(Vec2 alpha)
{
    Vec2 ret(0.0f, 0.0f);
    if (!_sprite) {
        return ret;
    }
    V3F_C4B_T2F_Quad quad = _sprite->getQuad();
    Vec2 min = Vec2(quad.bl.vertices.x,quad.bl.vertices.y);
    Vec2 max = Vec2(quad.tr.vertices.x,quad.tr.vertices.y);
    ret.x = min.x * (1.f - alpha.x) + max.x * alpha.x;
    ret.y = min.y * (1.f - alpha.y) + max.y * alpha.y;
    return ret;
}

void TurnableRadialProgressTimer::updateColor(void)
{
    if (!_sprite) {
        return;
    }
    
    if (_vertexData)
    {
        Color4B sc = _sprite->getQuad().tl.colors;
        for (int i = 0; i < _vertexDataCount; ++i)
        {
            _vertexData[i].colors = sc;
        }
    }
}

void TurnableRadialProgressTimer::setAnchorPoint(const Vec2& anchorPoint)
{
    Node::setAnchorPoint(anchorPoint);
}

Vec2 TurnableRadialProgressTimer::getMidpoint() const
{
    return _midpoint;
}

void TurnableRadialProgressTimer::setColor(const Color3B &color)
{
    if (_sprite != nullptr)
    {
        _sprite->setColor(color);
    }
    updateColor();
}

const Color3B& TurnableRadialProgressTimer::getColor() const
{
    if (_sprite != nullptr) {
        return _sprite->getColor();
    }
    return Color3B::BLACK;
}

void TurnableRadialProgressTimer::setOpacity(GLubyte opacity)
{
    if (_sprite != nullptr) {
        _sprite->setOpacity(opacity);
    }
    updateColor();
}

GLubyte TurnableRadialProgressTimer::getOpacity() const
{
    if (_sprite != nullptr) {
        return _sprite->getOpacity();
    }
    return 0;
}

void TurnableRadialProgressTimer::setMidpoint(const Vec2& midPoint)
{
    _midpoint = midPoint.getClampPoint(Vec2::ZERO, Vec2(1, 1));
}

///
//    Update does the work of mapping the texture onto the triangles
//    It now doesn't occur the cost of free/alloc data every update cycle.
//    It also only changes the percentage point but no other points if they have not
//    been modified.
//
//    It now deals with flipped texture. If you run into this problem, just use the
//    sprite property and enable the methods flipX, flipY.
///
void TurnableRadialProgressTimer::updateRadial(void)
{
    if (!_sprite) {
        return;
    }
    float alpha = _percentage / 100.f;
    
    float angle = 2.f*((float)M_PI) * ( _reverseDirection ? alpha : 1.0f - alpha);
    
    Vec2 startPoint;
    
    float baseAngle = _startAngle;
    while (baseAngle < 0.f) {
        baseAngle += 360.f;
    }
    
    std::vector<int> indeces;
    
    if (baseAngle < 90) {
        startPoint = Vec2(_midpoint.x, 1.f);
        indeces.push_back(0);
        indeces.push_back(1);
        indeces.push_back(2);
        indeces.push_back(3);
        indeces.push_back(4);
    }
    else if (baseAngle < 180) {
        startPoint = Vec2(1.f, _midpoint.y);
        indeces.push_back(1);
        indeces.push_back(2);
        indeces.push_back(3);
        indeces.push_back(4);
        indeces.push_back(0);
    }
    else if (baseAngle < 270) {
        startPoint = Vec2(_midpoint.x, 0.f);
        indeces.push_back(2);
        indeces.push_back(3);
        indeces.push_back(4);
        indeces.push_back(0);
        indeces.push_back(1);
    }
    else {
        startPoint = Vec2(0.f, _midpoint.y);
        indeces.push_back(3);
        indeces.push_back(4);
        indeces.push_back(0);
        indeces.push_back(1);
        indeces.push_back(2);
    }
    
    int firstIndex = indeces.front();
    int lastIndex = indeces.back();
    
    Vec2 topMid = startPoint; //Vec2(_midpoint.x, 1.f);
    Vec2 percentagePt = topMid.rotateByAngle(_midpoint, angle);
    
    CCLOG("startPoint: %f, %f", startPoint.x, startPoint.y);
    CCLOG("percentagePt: %f, %f", percentagePt.x, percentagePt.y);
    
    int index = 0;
    Vec2 hit = Vec2::ZERO;
    
    if (alpha == 0.f) {
        //    More efficient since we don't always need to check intersection
        //    If the alpha is zero then the hit point is top mid and the index is 0.
        hit = topMid;
        index = firstIndex;
    } else if (alpha == 1.f) {
        //    More efficient since we don't always need to check intersection
        //    If the alpha is one then the hit point is top mid and the index is 4.
        hit = topMid;
        index = lastIndex;
    } else {
        //    We run a for loop checking the edges of the texture to find the
        //    intersection point
        //    We loop through five points since the top is split in half
        
        float min_t = FLT_MAX;
        
//        for (int i = 0; i <= kProgressTextureCoordsCount; ++i) {
        for (int x = 0; x <= kProgressTextureCoordsCount; ++x) {
            int i = indeces[x];
            int pIndex = (i + (kProgressTextureCoordsCount - 1))%kProgressTextureCoordsCount;
            
            Vec2 edgePtA = boundaryTexCoord(i % kProgressTextureCoordsCount);
            Vec2 edgePtB = boundaryTexCoord(pIndex);
            
            //    Remember that the top edge is split in half for the 12 o'clock position
            //    Let's deal with that here by finding the correct endpoints
            float mid;
            if (baseAngle < 90) {
                mid = 1-_midpoint.x;
            }
            else if (baseAngle < 180) {
                mid = 1-_midpoint.y;
            }
            else if (baseAngle < 270) {
                mid = _midpoint.x;
            }
            else {
                mid = _midpoint.y;
            }
            
            if(i == firstIndex){
                edgePtB = edgePtA.lerp(edgePtB, mid);
            } else if(i == lastIndex){
                edgePtA = edgePtA.lerp(edgePtB, mid);
            }
            
            CCLOG("i=%d : A(%f, %f), B(%f, %f)", i, edgePtA.x, edgePtA.y, edgePtB.x, edgePtB.y);
            
            //    s and t are returned by ccpLineIntersect
            float s = 0, t = 0;
            if(Vec2::isLineIntersect(edgePtA, edgePtB, _midpoint, percentagePt, &s, &t))
            {
                
                //    Since our hit test is on rays we have to deal with the top edge
                //    being in split in half so we have to test as a segment
                if ((i == firstIndex || i == lastIndex)) {
                    //    s represents the point between edgePtA--edgePtB
                    if (!(0.f <= s && s <= 1.f)) {
                        continue;
                    }
                }
                //    As long as our t isn't negative we are at least finding a
                //    correct hitpoint from _midpoint to percentagePt.
                if (t >= 0.f) {
                    //    Because the percentage line and all the texture edges are
                    //    rays we should only account for the shortest intersection
                    if (t < min_t) {
                        min_t = t;
                        index = x;
                    }
                }
            }
        }
        
        //    Now that we have the minimum magnitude we can use that to find our intersection
        hit = _midpoint+ ((percentagePt - _midpoint) * min_t);
        
    }
    
    CCLOG("i0 = %d, i4 = %d", firstIndex, lastIndex);
    CCLOG("index: %d, %d", index, indeces[index]);
    CCLOG("hit: (%f, %f)", hit.x, hit.y);
    
    //    The size of the vertex data is the index from the hitpoint
    //    the 3 is for the _midpoint, 12 o'clock point and hitpoint position.
    
    bool sameIndexCount = true;
    int requiredIndexCount = index + 3;
    if(_vertexDataCount != requiredIndexCount){
        sameIndexCount = false;
        CC_SAFE_FREE(_vertexData);
        _vertexDataCount = 0;
    }
    
    
    if(!_vertexData) {
        _vertexDataCount = requiredIndexCount;
        _vertexData = (V2F_C4B_T2F*)malloc(_vertexDataCount * sizeof(V2F_C4B_T2F));
        CCASSERT( _vertexData, "CCTurnableRadialProgressTimer. Not enough memory");
    }
    updateColor();
    
    if (!sameIndexCount) {
        
        //    First we populate the array with the _midpoint, then all
        //    vertices/texcoords/colors of the 12 'o clock start and edges and the hitpoint
        _vertexData[0].texCoords = textureCoordFromAlphaPoint(_midpoint);
        _vertexData[0].vertices = vertexFromAlphaPoint(_midpoint);
        
        _vertexData[1].texCoords = textureCoordFromAlphaPoint(topMid);
        _vertexData[1].vertices = vertexFromAlphaPoint(topMid);
        
        for(int x = 0; x < index; x++){
            int i = indeces.at(x);
            Vec2 alphaPoint = boundaryTexCoord(i % kProgressTextureCoordsCount);
            CCLOG("(%d, %d) : (%f, %f)", x, i, alphaPoint.x, alphaPoint.y);
            _vertexData[x+2].texCoords = textureCoordFromAlphaPoint(alphaPoint);
            _vertexData[x+2].vertices = vertexFromAlphaPoint(alphaPoint);
        }
    }
    
    //    hitpoint will go last
    _vertexData[_vertexDataCount - 1].texCoords = textureCoordFromAlphaPoint(hit);
    _vertexData[_vertexDataCount - 1].vertices = vertexFromAlphaPoint(hit);
    
    CCLOG("vertex count: %d", _vertexDataCount);
    
    for (int x = 0; x < _vertexDataCount; x++)
    {
        auto v = _vertexData[x];
        CCLOG("vertex[%d] : {\n'vertices' : (%f, %f)\n}", x, v.vertices.x, v.vertices.y);
    }
}

Vec2 TurnableRadialProgressTimer::boundaryTexCoord(char index)
{
    if (index < kProgressTextureCoordsCount) {
        if (_reverseDirection) {
            return Vec2((kProgressTextureCoords>>(7-(index<<1)))&1,(kProgressTextureCoords>>(7-((index<<1)+1)))&1);
        } else {
            return Vec2((kProgressTextureCoords>>((index<<1)+1))&1,(kProgressTextureCoords>>(index<<1))&1);
        }
    }
    return Vec2::ZERO;
}

void TurnableRadialProgressTimer::onDraw(const Mat4 &transform, uint32_t flags)
{
    getGLProgram()->use();
    getGLProgram()->setUniformsForBuiltins(transform);
    
    GL::blendFunc( _sprite->getBlendFunc().src, _sprite->getBlendFunc().dst );
    
    GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX );
    
    GL::bindTexture2D( _sprite->getTexture()->getName() );
    
#ifdef EMSCRIPTEN
    setGLBufferData((void*) _vertexData, (_vertexDataCount * sizeof(V2F_C4B_T2F)), 0);
    
    int offset = 0;
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offset);
    
    offset += sizeof(Vec2);
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid*)offset);
    
    offset += sizeof(Color4B);
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid*)offset);
#else
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(_vertexData[0]) , &_vertexData[0].vertices);
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(_vertexData[0]), &_vertexData[0].texCoords);
    glVertexAttribPointer( GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(_vertexData[0]), &_vertexData[0].colors);
#endif // EMSCRIPTEN
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, _vertexDataCount);
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_vertexDataCount);
}

void TurnableRadialProgressTimer::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    if( ! _vertexData || ! _sprite)
        return;
    
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(TurnableRadialProgressTimer::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}


NS_CC_END