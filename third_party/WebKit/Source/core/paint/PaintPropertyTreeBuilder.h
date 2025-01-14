// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PaintPropertyTreeBuilder_h
#define PaintPropertyTreeBuilder_h

namespace blink {

class FrameView;
class LayoutObject;
struct PaintPropertyTreeBuilderContext;

// This class walks the whole layout tree, beginning from the root FrameView, across
// frame boundaries. The walk will collect special things in the layout tree and create
// paint property nodes for them. Special things include but not limit to: overflow clip,
// transform, fixed-pos, animation, mask, filter, ... etc.
// It expects to be invoked after layout clean, i.e. during InPaintPropertyUpdate phase.
class PaintPropertyTreeBuilder {
public:
    void buildPropertyTrees(FrameView& rootFrame);

private:
    void walk(FrameView&, const PaintPropertyTreeBuilderContext&);

    static void updatePaintOffsetTranslation(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateTransform(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateEffect(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateCssClip(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateLocalBorderBoxContext(LayoutObject&, const PaintPropertyTreeBuilderContext&);
    static void updateScrollbarPaintOffset(LayoutObject&, const PaintPropertyTreeBuilderContext&);
    static void updateOverflowClip(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updatePerspective(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateSvgLocalTransform(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateScrollTranslation(LayoutObject&, PaintPropertyTreeBuilderContext&);
    static void updateOutOfFlowContext(LayoutObject&, PaintPropertyTreeBuilderContext&);

    void walk(LayoutObject&, const PaintPropertyTreeBuilderContext&);
};

} // namespace blink

#endif // PaintPropertyTreeBuilder_h
