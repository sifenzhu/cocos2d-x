/****************************************************************************
 Copyright (c) 2014 Chukong Technologies Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CCPUParticle3DMaterialTranslator.h"
#include "3dparticle/ParticleUniverse/CCPUParticle3DMaterialManager.h"

NS_CC_BEGIN

enum MaterialToken
{
    TOKEN_MAT_TECHNIQUE,
    TOKEN_MAT_PASS,

    //pass
    TOKEN_MAT_LIGHTING,
    TOKEN_MAT_AMIBIENT,
    TOKEN_MAT_DIFFUSE,
    TOKEN_MAT_SPECULAR,
    TOKEN_MAT_EMISSIVE,
    TOKEN_MAT_BLEND,
    TOKEN_MAT_DEPTH_CHECK,
    TOKEN_MAT_DEPTH_WRITE,
    TOKEN_MAT_TEXTURE_UNIT,

    //texture unit
    TOKEN_MAT_TEXTURE_FILE,
    TOKEN_MAT_TEXTURE_WRAP,

    //status
    TOKEN_MAT_ON,
    TOKEN_MAT_OFF,
    TOKEN_MAT_BLEND_ADD,
    TOKEN_MAT_BLEND_ALPHA,
    TOKEN_MAT_BLEND_COLOR,
    TOKEN_MAT_BLEND_MODULATE,
    TOKEN_MAT_BLEND_REPLACE,
    TOKEN_MAT_BLEND_SRC_COLOR,
    TOKEN_MAT_BLEND_ONE,
    TOKEN_MAT_BLEND_ZERO,
    TOKEN_MAT_BLEND_DEST_COLOR,
    TOKEN_MAT_TEXTURE_REPEAT,
    TOKEN_MAT_TEXTURE_CLAMP,
    TOKEN_MAT_TEXTURE_MIRROR,
};

static const std::string matToken[] = {
    "technique",
    "pass",

    //pass
    "lighting",
    "ambient",
    "diffuse",
    "specular",
    "emissive",
    "scene_blend",
    "depth_check",
    "depth_write",
    "texture_unit",

    //texture unit
    "texture",
    "tex_address_mode",

    //status
    "on",
    "off",
    "add",
    "alpha_blend",
    "colour_blend",
    "modulate",
    "replace",
    "src_colour",
    "one",
    "zero",
    "dest_colour",
    "repeat",
    "clamp",
    "mirror",
};

PUParticle3DMaterialTranslator::PUParticle3DMaterialTranslator()//:mTechnique(0)
{
    
}
PUParticle3DMaterialTranslator::~PUParticle3DMaterialTranslator()
{
    
}

void PUParticle3DMaterialTranslator::translate(PUScriptCompiler* compiler, PUAbstractNode *node)
{
    PUObjectAbstractNode* obj = reinterpret_cast<PUObjectAbstractNode*>(node);
    PUObjectAbstractNode* parent = obj->parent ? reinterpret_cast<PUObjectAbstractNode*>(obj->parent) : 0;

    _material = new PUParticle3DMaterial();
    _material->fileName = obj->file;
    _material->name = obj->name;
    _material->autorelease();
    PUParticle3DMaterialCache::Instance()->addMaterial(_material);
    obj->context = _material;

    for(PUAbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
    {
        if((*i)->type == ANT_OBJECT)
        {
            PUObjectAbstractNode* child = reinterpret_cast<PUObjectAbstractNode*>(*i);
            if(child->cls == matToken[TOKEN_MAT_TECHNIQUE])
            {
                PUParticle3DMaterialTechniqueTranslator materialTechniqueTranslator;
                materialTechniqueTranslator.translate(compiler, *i);
            }
        }
    }
}


void PUParticle3DMaterialTechniqueTranslator::translate( PUScriptCompiler* compiler, PUAbstractNode *node )
{
    PUObjectAbstractNode* obj = reinterpret_cast<PUObjectAbstractNode*>(node);
    PUObjectAbstractNode* parent = obj->parent ? reinterpret_cast<PUObjectAbstractNode*>(obj->parent) : 0;

    if (parent)
        obj->context = parent->context;

    for(PUAbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
    {
        if((*i)->type == ANT_OBJECT)
        {
            PUObjectAbstractNode* child = reinterpret_cast<PUObjectAbstractNode*>(*i);
            if(child->cls == matToken[TOKEN_MAT_PASS])
            {
                PUParticle3DMaterialPassTranslator materialPassTranslator;
                materialPassTranslator.translate(compiler, *i);
            }
        }
    }
}


void PUParticle3DMaterialPassTranslator::translate( PUScriptCompiler* compiler, PUAbstractNode *node )
{
    PUObjectAbstractNode* obj = reinterpret_cast<PUObjectAbstractNode*>(node);
    PUObjectAbstractNode* parent = obj->parent ? reinterpret_cast<PUObjectAbstractNode*>(obj->parent) : 0;

    if (parent)
        obj->context = parent->context;

    PUParticle3DMaterial *material = static_cast<PUParticle3DMaterial *>(obj->context);

    for(PUAbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
    {
        if((*i)->type == ANT_PROPERTY)
        {
            PUPropertyAbstractNode *prop = reinterpret_cast<PUPropertyAbstractNode*>((*i));
            if (prop->name == matToken[TOKEN_MAT_LIGHTING])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_LIGHTING], VAL_STRING))
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        if (val == matToken[TOKEN_MAT_ON])
                        {
                            material->isEnabledLight = true;
                        }
                        else if (val == matToken[TOKEN_MAT_OFF])
                        {
                            material->isEnabledLight = false;
                        }
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_AMIBIENT])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_AMIBIENT], VAL_VECTOR4))
                {
                    Vec4 val;
                    if(getVector4(prop->values.begin(), prop->values.end(), &val))
                    {
                        material->ambientColor = val;
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_DIFFUSE])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_AMIBIENT], VAL_VECTOR4))
                {
                    Vec4 val;
                    if(getVector4(prop->values.begin(), prop->values.end(), &val))
                    {
                        material->diffuseColor = val;
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_SPECULAR])
            {
                PUAbstractNodeList::const_iterator i = prop->values.begin();
                PUAbstractNodeList::const_iterator end = prop->values.end();
                unsigned int n = 0;
                Vec4 color;
                float shininess;
                while(i != end)
                {
                    float v = 0;
                    if(getFloat(**i, &v))
                    {
                        switch(n)
                        {
                        case 0:
                            color.x = v;
                            break;
                        case 1:
                            color.y = v;
                            break;
                        case 2:
                            color.z = v;
                            break;
                        case 3:
                            color.w = v;
                            break;
                        case 4:
                            shininess = v;
                            break;
                        }
                    }
                    ++n;
                    ++i;
                }

                material->specularColor = color;
                material->shininess = shininess;
            }
            else if (prop->name == matToken[TOKEN_MAT_EMISSIVE])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_AMIBIENT], VAL_VECTOR4))
                {
                    Vec4 val;
                    if(getVector4(prop->values.begin(), prop->values.end(), &val))
                    {
                        material->emissiveColor = val;
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_BLEND])
            {
                //if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_BLEND], VAL_STRING))
                if (!prop->values.empty())
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        if (val == matToken[TOKEN_MAT_BLEND_ADD])
                        {
                            material->blendFunc.src = GL_ONE;
                            material->blendFunc.dst= GL_ONE;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_ALPHA])
                        {
                            material->blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_COLOR])
                        {
                            material->blendFunc.src = GL_SRC_COLOR;
                            material->blendFunc.dst= GL_ONE_MINUS_SRC_COLOR;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_MODULATE])
                        {
                            material->blendFunc.src = GL_DST_COLOR;
                            material->blendFunc.dst= GL_ZERO;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_REPLACE])
                        {
                            material->blendFunc.src = GL_ONE;
                            material->blendFunc.dst= GL_ZERO;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_SRC_COLOR])
                        {
                            material->blendFunc.src = GL_SRC_COLOR;
                        }
                        else if (val == matToken[TOKEN_MAT_BLEND_DEST_COLOR])
                        {
                            material->blendFunc.src = GL_DST_COLOR;
                        }
                    }

                    if (prop->values.size() == 2)
                    {
                        if (getString(*prop->values.back(), &val))
                        {
                            if (val == matToken[TOKEN_MAT_BLEND_ONE])
                            {
                                material->blendFunc.dst = GL_ONE;
                            }
                            else if (val == matToken[TOKEN_MAT_BLEND_ZERO])
                            {
                                material->blendFunc.dst = GL_ZERO;
                            }
                            else if (val == matToken[TOKEN_MAT_BLEND_SRC_COLOR])
                            {
                                material->blendFunc.dst = GL_SRC_COLOR;
                            }
                            else if (val == matToken[TOKEN_MAT_BLEND_DEST_COLOR])
                            {
                                material->blendFunc.dst = GL_DST_COLOR;
                            }
                        }
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_DEPTH_CHECK])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_DEPTH_CHECK], VAL_STRING))
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        if (val == matToken[TOKEN_MAT_ON])
                        {
                            material->depthTest = true;
                        }
                        else if (val == matToken[TOKEN_MAT_OFF])
                        {
                            material->depthTest = false;
                        }
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_DEPTH_WRITE])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_DEPTH_WRITE], VAL_STRING))
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        if (val == matToken[TOKEN_MAT_ON])
                        {
                            material->depthWrite = true;
                        }
                        else if (val == matToken[TOKEN_MAT_OFF])
                        {
                            material->depthWrite = false;
                        }
                    }
                }
            }
        }
        else if ((*i)->type == ANT_OBJECT)
        {
            PUObjectAbstractNode* child = reinterpret_cast<PUObjectAbstractNode*>(*i);
            if(child->cls == matToken[TOKEN_MAT_TEXTURE_UNIT])
            {
                PUParticle3DMaterialTextureUnitTranslator materialTextureUnitTranslator;
                materialTextureUnitTranslator.translate(compiler, *i);
            }
        }
    }
}


void PUParticle3DMaterialTextureUnitTranslator::translate( PUScriptCompiler* compiler, PUAbstractNode *node )
{
    PUObjectAbstractNode* obj = reinterpret_cast<PUObjectAbstractNode*>(node);
    PUObjectAbstractNode* parent = obj->parent ? reinterpret_cast<PUObjectAbstractNode*>(obj->parent) : 0;

    PUParticle3DMaterial *material = static_cast<PUParticle3DMaterial *>(parent->context);

    for(PUAbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
    {
        if((*i)->type == ANT_PROPERTY)
        {
            PUPropertyAbstractNode *prop = reinterpret_cast<PUPropertyAbstractNode*>((*i));
            if (prop->name == matToken[TOKEN_MAT_TEXTURE_FILE])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_TEXTURE_FILE], VAL_STRING))
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        material->textureFile = val;
                    }
                }
            }
            else if (prop->name == matToken[TOKEN_MAT_TEXTURE_WRAP])
            {
                if (passValidateProperty(compiler, prop, matToken[TOKEN_MAT_TEXTURE_WRAP], VAL_STRING))
                {
                    std::string val;
                    if(getString(*prop->values.front(), &val))
                    {
                        if (val == matToken[TOKEN_MAT_TEXTURE_CLAMP])
                        {
                            material->wrapMode = GL_CLAMP_TO_EDGE;
                        }
                        else if (val == matToken[TOKEN_MAT_TEXTURE_REPEAT])
                        {
                            material->wrapMode = GL_REPEAT;
                        }
                        else if (val == matToken[TOKEN_MAT_TEXTURE_MIRROR])
                        {
                            material->wrapMode = GL_MIRRORED_REPEAT;//GL_MIRROR_CLAMP_EXT;
                        }
                    }
                }
            }
        }
    }
}

NS_CC_END