#include <mbgl/test/util.hpp>

#include <mbgl/renderer/circle_bucket.hpp>
#include <mbgl/renderer/fill_bucket.hpp>
#include <mbgl/renderer/line_bucket.hpp>
#include <mbgl/renderer/symbol_bucket.hpp>

#include <mbgl/style/layers/symbol_layer_properties.hpp>

#include <mbgl/map/mode.hpp>

TEST(Buckets, CircleBucket) {
    mbgl::style::CirclePaintProperties::Evaluated properties;
    mbgl::MapMode mapMode = mbgl::MapMode::Still;

    mbgl::CircleBucket bucket { properties, 0, mapMode };
    ASSERT_FALSE(bucket.hasData());
}

TEST(Buckets, FillBucket) {
    mbgl::style::FillPaintProperties::Evaluated properties;

    mbgl::FillBucket bucket { properties, 0 };
    ASSERT_FALSE(bucket.hasData());
}

TEST(Buckets, LineBucket) {
    mbgl::style::LinePaintProperties::Evaluated properties;
    uint32_t overscaling = 0;

    mbgl::LineBucket bucket { properties, 0, overscaling };
    ASSERT_FALSE(bucket.hasData());
}

TEST(Buckets, SymbolBucket) {
    mbgl::MapMode mapMode = mbgl::MapMode::Still;
    mbgl::style::SymbolPaintProperties::Evaluated paint;
    mbgl::style::SymbolLayoutProperties::Evaluated layout;
    bool sdfIcons = false;
    bool iconsNeedLinear = false;

    mbgl::SymbolBucket bucket { mapMode, layout, paint, 0, sdfIcons, iconsNeedLinear };
    ASSERT_FALSE(bucket.hasIconData());
    ASSERT_FALSE(bucket.hasTextData());
    ASSERT_FALSE(bucket.hasCollisionBoxData());
}
