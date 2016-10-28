#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/style/bucket_parameters.hpp>
#include <mbgl/renderer/fill_bucket.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>

namespace mbgl {
namespace style {

void FillLayer::Impl::cascade(const CascadeParameters& parameters) {
    paint.cascade(parameters);
}

bool FillLayer::Impl::evaluate(const PropertyEvaluationParameters& parameters) {
    paint.evaluate(parameters);

    if (paint.unevaluated.get<FillOutlineColor>().isUndefined()) {
        paint.evaluated.get<FillOutlineColor>() = paint.evaluated.get<FillColor>();
    }

    passes = RenderPass::None;

    if (paint.evaluated.get<FillAntialias>()) {
        passes |= RenderPass::Translucent;
    }

    if (!paint.unevaluated.get<FillPattern>().isUndefined()
      || paint.evaluated.get<FillColor>().constantOr(Color()).a < 1.0f
      || paint.evaluated.get<FillOpacity>().constantOr(0) < 1.0f) {
        passes |= RenderPass::Translucent;
    } else {
        passes |= RenderPass::Opaque;
    }

    return paint.hasTransition();
}

std::unique_ptr<Bucket> FillLayer::Impl::createBucket(BucketParameters& parameters, const GeometryTileLayer& layer) const {
    auto bucket = std::make_unique<FillBucket>(paint.evaluated, parameters.tileID.overscaledZ);

    parameters.eachFilteredFeature(filter, layer, [&] (const auto& feature, std::size_t index, const std::string& layerName) {
        auto geometries = feature.getGeometries();
        bucket->addFeature(feature, geometries);
        parameters.featureIndex.insert(geometries, index, layerName, id);
    });

    return std::move(bucket);
}

float FillLayer::Impl::getQueryRadius() const {
    const std::array<float, 2>& translate = paint.evaluated.get<FillTranslate>();
    return util::length(translate[0], translate[1]);
}

bool FillLayer::Impl::queryIntersectsGeometry(
        const GeometryCoordinates& queryGeometry,
        const GeometryCollection& geometry,
        const float bearing,
        const float pixelsToTileUnits) const {

    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(
            queryGeometry, paint.evaluated.get<FillTranslate>(), paint.evaluated.get<FillTranslateAnchor>(), bearing, pixelsToTileUnits);

    return util::polygonIntersectsMultiPolygon(translatedQueryGeometry.value_or(queryGeometry), geometry);
}

} // namespace style
} // namespace mbgl
