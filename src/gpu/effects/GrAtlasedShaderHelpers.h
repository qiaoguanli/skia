/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasedShaderHelpers_DEFINED
#define GrAtlasedShaderHelpers_DEFINED

#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

static void append_index_uv_varyings(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                     const char* inTexCoordsName,
                                     const char* atlasSizeInvName,
                                     GrGLSLVertToFrag *uv,
                                     GrGLSLVertToFrag *texIdx,
                                     GrGLSLVertToFrag *st) {
    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: texel coordinates are multiplied by 2 (or shifted left 1)
    //                    texture index is stored as lower bits of both x and y
    args.fVertBuilder->codeAppendf("float2 indexTexCoords = float2(%s.x, %s.y);",
                                   inTexCoordsName, inTexCoordsName);
    args.fVertBuilder->codeAppend("float2 intCoords = floor(0.5*indexTexCoords);");
    args.fVertBuilder->codeAppend("float2 diff = indexTexCoords - 2.0*intCoords;");
    args.fVertBuilder->codeAppend("float texIdx = 2.0*diff.x + diff.y;");

    // Multiply by 1/atlasSize to get normalized texture coordinates
    args.fVaryingHandler->addVarying("TextureCoords", uv, kHigh_GrSLPrecision);
    args.fVertBuilder->codeAppendf("%s = intCoords * %s;", uv->vsOut(), atlasSizeInvName);

    args.fVaryingHandler->addVarying("TexIndex", texIdx);
    args.fVertBuilder->codeAppendf("%s = texIdx;", texIdx->vsOut());

    if (st) {
        args.fVaryingHandler->addVarying("IntTextureCoords", st, kHigh_GrSLPrecision);
        args.fVertBuilder->codeAppendf("%s = intCoords;", st->vsOut());
    }
}

static void append_multitexture_lookup(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                       int numTextureSamplers,
                                       const GrGLSLVertToFrag &texIdx,
                                       const char* coordName,
                                       const char* colorName) {
    // conditionally load from the indexed texture sampler
    if (numTextureSamplers > 1) {
        args.fFragBuilder->codeAppendf("if (%s == 0) ", texIdx.fsIn());
    }
    args.fFragBuilder->codeAppendf("{ %s = ", colorName);
    args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], coordName, kVec2f_GrSLType);
    args.fFragBuilder->codeAppend("; }");
    for (int i = 1; i < numTextureSamplers; ++i) {
        args.fFragBuilder->codeAppendf("else if (%s == %d) { %s =", texIdx.fsIn(), i, colorName);
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[i], coordName, kVec2f_GrSLType);
        args.fFragBuilder->codeAppend("; }");
    }
}

#endif
