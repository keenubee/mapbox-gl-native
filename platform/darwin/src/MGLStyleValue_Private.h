#import <Foundation/Foundation.h>

#import "MGLStyleValue.h"

#import "NSValue+MGLStyleAttributeAdditions.h"
#import "MGLTypes.h"

#import <mbgl/util/enum.hpp>

#include <mbgl/style/data_driven_property_value.hpp>

#if TARGET_OS_IPHONE
    #import "UIColor+MGLAdditions.h"
#else
    #import "NSColor+MGLAdditions.h"
#endif

template <typename MBGLType, typename ObjCType, typename MBGLElement = MBGLType, typename ObjCEnum = ObjCType>
class MGLStyleValueTransformer {
public:

    // Convert an mbgl property value into an mgl style value
    MGLStyleValue<ObjCType> *toStyleValue(const mbgl::style::PropertyValue<MBGLType> &mbglValue) {
        PropertyValueEvaluator evaluator;
        return mbglValue.evaluate(evaluator);
    }

    // Convert an mbgl data driven property value into an mgl style value
    MGLStyleValue<ObjCType> *toDataDrivenStyleValue(const mbgl::style::DataDrivenPropertyValue<MBGLType> &mbglValue) {
        PropertyValueEvaluator evaluator;
        return mbglValue.evaluate(evaluator);
    }

    // Convert an mbgl property value containing an enum into an mgl style value
    template <typename MBGLEnum = MBGLType,
              class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
              typename MGLEnum = ObjCEnum,
              class = typename std::enable_if<std::is_enum<MGLEnum>::value>::type>
    MGLStyleValue<ObjCType> *toEnumStyleValue(const mbgl::style::PropertyValue<MBGLEnum> &mbglValue) {
        EnumPropertyValueEvaluator<MBGLEnum, ObjCEnum> evaluator;
        return mbglValue.evaluate(evaluator);
    }

    // Convert an mgl style value into a mgl property value
    mbgl::style::PropertyValue<MBGLType> toPropertyValue(MGLStyleValue<ObjCType> *value) {
        if ([value isKindOfClass:[MGLStyleConstantValue class]]) {
            MBGLType mbglValue;
            id mglValue = [(MGLStyleConstantValue<ObjCType> *)value rawValue];
            getMBGLValue(mglValue, mbglValue);
            return mbglValue;
        } else if (value) {
            [NSException raise:@"MGLAbstractClassException" format:
             @"The style value %@ cannot be applied to the style. "
             @"Make sure the style value was created as a member of a concrete subclass of MGLStyleValue.",
             NSStringFromClass([value class])];
            return {};
        } else {
            return {};
        }
    }

    // Convert an mgl style value into a mgl data driven property value
    mbgl::style::DataDrivenPropertyValue<MBGLType> toDataDrivenPropertyValue(MGLStyleValue<ObjCType> *value) {
        if ([value isKindOfClass:[MGLStyleConstantValue class]]) {
            MBGLType mbglValue;
            id mglValue = [(MGLStyleConstantValue<ObjCType> *)value rawValue];
            getMBGLValue(mglValue, mbglValue);
            return mbglValue;
        } else if ([value isKindOfClass:[MGLStyleFunction class]]) {
            MGLStyleFunction<ObjCType> *function = (MGLStyleFunction<ObjCType> *)value;

            __block std::map<float, MBGLType> stops = {};
            [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<ObjCType> * _Nonnull stopValue, BOOL * _Nonnull stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                auto mbglStopValue = toPropertyValue(stopValue);
                NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");
                stops[zoomKey.floatValue] = mbglStopValue.asConstant();
            }];

            // Camera function with Exponential stops
            mbgl::style::ExponentialStops<MBGLType> exponentialStops = {stops, (float)function.base};
            mbgl::style::CameraFunction<MBGLType> cameraFunction = {exponentialStops};

            return cameraFunction;
        } else if ([value isKindOfClass:[MGLStyleIntervalFunction class]]) {
            MGLStyleIntervalFunction<ObjCType> *function = (MGLStyleIntervalFunction<ObjCType> *)value;

            __block std::map<float, MBGLType> stops = {};
            [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<ObjCType> * _Nonnull stopValue, BOOL * _Nonnull stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                auto mbglStopValue = toPropertyValue(stopValue);
                NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");
                stops[zoomKey.floatValue] = mbglStopValue.asConstant();
            }];

            // Camera function with Interval stops
            mbgl::style::IntervalStops<MBGLType> intervalStops = {stops};
            mbgl::style::CameraFunction<MBGLType> cameraFunction = {intervalStops};

            return cameraFunction;
        } else if ([value isKindOfClass:[MGLStyleSourceCategoricalFunction class]]) {
            MGLStyleSourceCategoricalFunction<ObjCType> *function = (MGLStyleSourceCategoricalFunction<ObjCType> *)value;

            __block std::map<mbgl::style::CategoricalValue, MBGLType> stops = {};

            [function.stops enumerateKeysAndObjectsUsingBlock:^(id categoryKey, MGLStyleValue<ObjCType> *stopValue, BOOL *stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                auto mbglStopValue = toPropertyValue(stopValue);
                NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");

                if ([categoryKey isKindOfClass:[NSString class]]) {
                    const std::string& convertedValueKey = [((NSString *)categoryKey) UTF8String];
                    stops[mbgl::style::CategoricalValue(convertedValueKey)] = mbglStopValue.asConstant();
                } else if ([categoryKey isKindOfClass:[NSNumber class]]) {
                    NSNumber *key = (NSNumber *)categoryKey;
                    if ((strcmp([key objCType], @encode(char)) == 0) ||
                        (strcmp([key objCType], @encode(BOOL)) == 0)) {
                        stops[mbgl::style::CategoricalValue((bool)[key boolValue])] = mbglStopValue.asConstant();
                    } else if (strcmp([key objCType], @encode(double)) == 0 ||
                               strcmp([key objCType], @encode(float)) == 0) {
                        NSCAssert(mbglStopValue.isConstant(), @"Categorical stop keys must be strings, booleans, or integers");
                    } else if ([key compare:@(0)] == NSOrderedDescending ||
                               [key compare:@(0)] == NSOrderedSame ||
                               [key compare:@(0)] == NSOrderedAscending) {
                        stops[mbgl::style::CategoricalValue((int64_t)[key integerValue])] = mbglStopValue.asConstant();
                    }
                }
            }];
            mbgl::style::CategoricalStops<MBGLType> categoricalStops = {stops};
            if (function.defaultValue) {
                NSCAssert([function.defaultValue isKindOfClass:[MGLStyleConstantValue class]], @"Default value must be constant");
                MBGLType mbglValue;
                id mglValue = [(MGLStyleConstantValue<ObjCType> *)function.defaultValue rawValue];
                getMBGLValue(mglValue, mbglValue);
                categoricalStops.defaultValue = mbglValue;
            }
            mbgl::style::SourceFunction<MBGLType> sourceFunction = {function.attributeName.UTF8String, categoricalStops};
            return sourceFunction;
        } else if ([value isKindOfClass:[MGLStyleSourceExponentialFunction class]]) {
            MGLStyleSourceExponentialFunction<ObjCType> *function = (MGLStyleSourceExponentialFunction<ObjCType> *)value;
            __block std::map<float, MBGLType> stops = {};
            [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<ObjCType> * _Nonnull stopValue, BOOL * _Nonnull stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                auto mbglStopValue = toPropertyValue(stopValue);
                NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");
                stops[zoomKey.floatValue] = mbglStopValue.asConstant();
            }];
            mbgl::style::ExponentialStops<MBGLType> exponentialStops = {stops, (float)function.base};
            mbgl::style::SourceFunction<MBGLType> sourceFunction = {function.attributeName.UTF8String, exponentialStops};
            return sourceFunction;
        } else if ([value isKindOfClass:[MGLStyleSourceIntervalFunction class]]) {
            MGLStyleSourceIntervalFunction<ObjCType> *function = (MGLStyleSourceIntervalFunction<ObjCType> *)value;
            __block std::map<float, MBGLType> stops = {};
            [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<ObjCType> * _Nonnull stopValue, BOOL * _Nonnull stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                auto mbglStopValue = toPropertyValue(stopValue);
                NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");
                stops[zoomKey.floatValue] = mbglStopValue.asConstant();
            }];
            mbgl::style::IntervalStops<MBGLType> intervalStops = {stops};
            mbgl::style::SourceFunction<MBGLType> sourceFunction = {function.attributeName.UTF8String, intervalStops};
            return sourceFunction;
        } else if ([value isKindOfClass:[MGLStyleSourceIdentityFunction class]]) {
            MGLStyleSourceIdentityFunction<ObjCType> *function = (MGLStyleSourceIdentityFunction<ObjCType> *)value;
            mbgl::style::IdentityStops<MBGLType> identityStops;
            mbgl::style::SourceFunction<MBGLType> sourceFunction = {function.attributeName.UTF8String, identityStops};
            return sourceFunction;
        } else if ([value isKindOfClass:[MGLStyleCompositeCategoricalFunction class]]) {
            MGLStyleCompositeCategoricalFunction<ObjCType> *function = (MGLStyleCompositeCategoricalFunction<ObjCType> *)value;
            __block std::map<float, mbgl::style::CategoricalStops<MBGLType>> compositeStops = {};
            [function.stops enumerateKeysAndObjectsUsingBlock:^(NSNumber * _Nonnull zoomKey, MGLStyleValue<ObjCType> * _Nonnull stopValue, BOOL * _Nonnull stop) {
                NSCAssert([stopValue isKindOfClass:[MGLStyleSourceCategoricalFunction class]], @"Stops should be MGLStyleSourceCategoricalFunction");

                MGLStyleSourceCategoricalFunction<ObjCType> *innerFunction = (MGLStyleSourceCategoricalFunction<ObjCType> *)stopValue;
                __block std::map<mbgl::style::CategoricalValue, MBGLType> stops = {};

                [innerFunction.stops enumerateKeysAndObjectsUsingBlock:^(id categoryKey, MGLStyleValue<ObjCType> *innerStopValue, BOOL *innerStop) {
                    NSCAssert([innerStopValue isKindOfClass:[MGLStyleValue class]], @"Stops should be MGLStyleValues");
                    auto mbglStopValue = toPropertyValue(innerStopValue);
                    NSCAssert(mbglStopValue.isConstant(), @"Stops must be constant");

                    if ([categoryKey isKindOfClass:[NSString class]]) {
                        const std::string& convertedValueKey = [((NSString *)categoryKey) UTF8String];
                        stops[mbgl::style::CategoricalValue(convertedValueKey)] = mbglStopValue.asConstant();
                    } else if ([categoryKey isKindOfClass:[NSNumber class]]) {
                        NSNumber *key = (NSNumber *)categoryKey;
                        if ((strcmp([key objCType], @encode(char)) == 0) ||
                            (strcmp([key objCType], @encode(BOOL)) == 0)) {
                            stops[mbgl::style::CategoricalValue((bool)[key boolValue])] = mbglStopValue.asConstant();
                        } else if (strcmp([key objCType], @encode(double)) == 0 ||
                                   strcmp([key objCType], @encode(float)) == 0) {
                            NSCAssert(mbglStopValue.isConstant(), @"Categorical stop keys must be strings, booleans, or integers");
                        } else if ([key compare:@(0)] == NSOrderedDescending ||
                                   [key compare:@(0)] == NSOrderedSame ||
                                   [key compare:@(0)] == NSOrderedAscending) {
                            stops[mbgl::style::CategoricalValue((int64_t)[key integerValue])] = mbglStopValue.asConstant();
                        }
                    }
                }];
                mbgl::style::CategoricalStops<MBGLType> categoricalStops = {stops};
                if (innerFunction.defaultValue) {
                    NSCAssert([innerFunction.defaultValue isKindOfClass:[MGLStyleConstantValue class]], @"Default value must be constant");
                    MBGLType mbglValue;
                    id mglValue = [(MGLStyleConstantValue<ObjCType> *)innerFunction.defaultValue rawValue];
                    getMBGLValue(mglValue, mbglValue);
                    categoricalStops.defaultValue = mbglValue;
                }
                compositeStops[zoomKey.floatValue] = categoricalStops;
            }];
            mbgl::style::CompositeFunction<MBGLType> compositeFunction = {function.attributeName.UTF8String, compositeStops};
            return compositeFunction;
        } else if (value) {
            [NSException raise:@"MGLAbstractClassException" format:
             @"The style value %@ cannot be applied to the style. "
             @"Make sure the style value was created as a member of a concrete subclass of MGLStyleValue.",
             NSStringFromClass([value class])];
            return {};
        } else {
            return {};
        }
    }

    // Convert an mgl style value containing an enum into a mgl property value containing an enum
    template <typename MBGLEnum = MBGLType,
              class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
              typename MGLEnum = ObjCEnum,
              class = typename std::enable_if<std::is_enum<MGLEnum>::value>::type>
    mbgl::style::PropertyValue<MBGLEnum> toEnumPropertyValue(MGLStyleValue<ObjCType> *value) {
        if ([value isKindOfClass:[MGLStyleConstantValue class]]) {
            MBGLEnum mbglValue;
            getMBGLValue([(MGLStyleConstantValue<ObjCType> *)value rawValue], mbglValue);
            return mbglValue;
        } else if (value) {
            [NSException raise:@"MGLAbstractClassException" format:
             @"The style value %@ cannot be applied to the style. "
             @"Make sure the style value was created as a member of a concrete subclass of MGLStyleValue.",
             NSStringFromClass([value class])];
            return {};
        } else {
            return {};
        }
    }

private: // Private utilities for converting from mgl to mbgl values

    // Bool
    void getMBGLValue(NSNumber *rawValue, bool &mbglValue) {
        mbglValue = !!rawValue.boolValue;
    }

    // Float
    void getMBGLValue(NSNumber *rawValue, float &mbglValue) {
        mbglValue = rawValue.floatValue;
    }

    // String
    void getMBGLValue(NSString *rawValue, std::string &mbglValue) {
        mbglValue = rawValue.UTF8String;
    }
    
    // Offsets
    void getMBGLValue(NSValue *rawValue, std::array<float, 2> &mbglValue) {
        mbglValue = rawValue.mgl_offsetArrayValue;
    }
    
    // Padding
    void getMBGLValue(NSValue *rawValue, std::array<float, 4> &mbglValue) {
        mbglValue = rawValue.mgl_paddingArrayValue;
    }

    // Color
    void getMBGLValue(MGLColor *rawValue, mbgl::Color &mbglValue) {
        mbglValue = rawValue.mgl_color;
    }

    // Array
    void getMBGLValue(ObjCType rawValue, std::vector<MBGLElement> &mbglValue) {
        mbglValue.reserve(rawValue.count);
        for (id obj in rawValue) {
            MBGLElement mbglElement;
            getMBGLValue(obj, mbglElement);
            mbglValue.push_back(mbglElement);
        }
    }

    // Enumerations
    template <typename MBGLEnum = MBGLType,
    class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
    typename MGLEnum = ObjCEnum,
    class = typename std::enable_if<std::is_enum<MGLEnum>::value>::type>
    void getMBGLValue(ObjCType rawValue, MBGLEnum &mbglValue) {
        MGLEnum mglEnum;
        [rawValue getValue:&mglEnum];
        auto str = mbgl::Enum<MGLEnum>::toString(mglEnum);
        mbglValue = *mbgl::Enum<MBGLEnum>::toEnum(str);
    }

private: // Private utilities for converting from mbgl to mgl values

    // Bool
    static NSNumber *toMGLRawStyleValue(const bool mbglStopValue) {
        return @(mbglStopValue);
    }

    // Float
    static NSNumber *toMGLRawStyleValue(const float mbglStopValue) {
        return @(mbglStopValue);
    }

    // Integer
    static NSNumber *toMGLRawStyleValue(const int64_t mbglStopValue) {
        return @(mbglStopValue);
    }

    // String
    static NSString *toMGLRawStyleValue(const std::string &mbglStopValue) {
        return @(mbglStopValue.c_str());
    }

    // Offsets
    static NSValue *toMGLRawStyleValue(const std::array<float, 2> &mbglStopValue) {
        return [NSValue mgl_valueWithOffsetArray:mbglStopValue];
    }

    // Padding
    static NSValue *toMGLRawStyleValue(const std::array<float, 4> &mbglStopValue) {
        return [NSValue mgl_valueWithPaddingArray:mbglStopValue];
    }

    // Color
    static MGLColor *toMGLRawStyleValue(const mbgl::Color mbglStopValue) {
        return [MGLColor mgl_colorWithColor:mbglStopValue];
    }

    // Array
    static ObjCType toMGLRawStyleValue(const std::vector<MBGLElement> &mbglStopValue) {
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:mbglStopValue.size()];
        for (const auto &mbglElement: mbglStopValue) {
            [array addObject:toMGLRawStyleValue(mbglElement)];
        }
        return array;
    }

    // Enumerations
    template <typename MBGLEnum = MBGLType, typename MGLEnum = ObjCEnum>
    static NSValue *toMGLRawStyleValue(const MBGLEnum &value) {
        auto str = mbgl::Enum<MBGLEnum>::toString(value);
        MGLEnum mglType = *mbgl::Enum<MGLEnum>::toEnum(str);
        return [NSValue value:&mglType withObjCType:@encode(MGLEnum)];
    }

    // Converts mbgl stops to an equivilant NSDictionary for mgl
    static NSMutableDictionary *toConvertedStops(const std::map<float, MBGLType> &mbglStops) {
        NSMutableDictionary *stops = [NSMutableDictionary dictionaryWithCapacity:mbglStops.size()];
        for (const auto &mbglStop : mbglStops) {
            auto rawValue = toMGLRawStyleValue(mbglStop.second);
            stops[@(mbglStop.first)] = [MGLStyleValue valueWithRawValue:rawValue];
        }
        return stops;
    }

    // Converts mbgl interval stop categorical values to an equivilant object for mgl
    class CategoricalValueVisitor {
    public:
        id operator()(const bool value) {
            return toMGLRawStyleValue(value);
        }

        id operator()(const int64_t value) {
            return toMGLRawStyleValue(value);
        }

        id operator()(const std::string value) {
            return toMGLRawStyleValue(value);
        }
    };

    // Converts a source function and all possible mbgl source function stops into an equivilant mgl style value
    class SourceFunctionStopsVisitor {
    public:
        id operator()(const mbgl::style::ExponentialStops<MBGLType> &mbglStops) {
            return [MGLStyleSourceExponentialFunction functionWithAttributeName:@(mbglFunction.property.c_str())
                                                                           base:mbglStops.base
                                                               exponentialStops:toConvertedStops(mbglStops.stops)];
        }

        id operator()(const mbgl::style::IntervalStops<MBGLType> &mbglStops) {
            return [MGLStyleSourceIntervalFunction functionWithAttributeName:@(mbglFunction.property.c_str())
                                                               intervalStops:toConvertedStops(mbglStops.stops)];
        }

        id operator()(const mbgl::style::CategoricalStops<MBGLType> &mbglStops) {
            NSMutableDictionary *stops = [NSMutableDictionary dictionaryWithCapacity:mbglStops.stops.size()];
            for (const auto &mbglStop : mbglStops.stops) {
                auto categoricalValue = mbglStop.first;
                auto rawValue = toMGLRawStyleValue(mbglStop.second);
                CategoricalValueVisitor categoricalValueVisitor;
                id stopKey = apply_visitor(categoricalValueVisitor, categoricalValue);
                stops[stopKey] = [MGLStyleValue valueWithRawValue:rawValue];
            }
            MGLStyleValue<ObjCType> *defaultValue = [MGLStyleValue valueWithRawValue:toMGLRawStyleValue(mbglStops.defaultValue)];
            return [MGLStyleSourceCategoricalFunction functionWithAttributeName:@(mbglFunction.property.c_str()) categoricalStops:stops defaultValue:defaultValue];
        }

        id operator()(const mbgl::style::IdentityStops<MBGLType> &mbglStops) {
            return [MGLStyleSourceIdentityFunction functionWithAttributeName:@(mbglFunction.property.c_str())];
        }
        
        const mbgl::style::SourceFunction<MBGLType> &mbglFunction;
    };

    // Converts all possible mbgl camera function stops into an equivilant mgl style value
    class CameraFunctionStopsVisitor {
    public:
        id operator()(const mbgl::style::ExponentialStops<MBGLType> &mbglStops) {
            return [MGLStyleFunction functionWithBase:mbglStops.base stops:toConvertedStops(mbglStops.stops)];
        }

        id operator()(const mbgl::style::IntervalStops<MBGLType> &mbglStops) {
            return [MGLStyleIntervalFunction functionWithIntervalStops:toConvertedStops(mbglStops.stops)];
        }
    };

    // Converts all types of mbgl property values containing enumerations into an equivilant mgl style value
    template <typename MBGLEnum = MBGLType, typename MGLEnum = ObjCEnum>
    class EnumPropertyValueEvaluator {
    public:
        id operator()(const mbgl::style::Undefined) const {
            return nil;
        }

        id operator()(const MBGLEnum &value) const {
            auto str = mbgl::Enum<MBGLEnum>::toString(value);
            MGLEnum mglType = *mbgl::Enum<MGLEnum>::toEnum(str);
            return [MGLStyleConstantValue<ObjCType> valueWithRawValue:[NSValue value:&mglType withObjCType:@encode(MGLEnum)]];
        }

        id operator()(const mbgl::style::CameraFunction<MBGLEnum> &mbglValue) const {
            CameraFunctionStopsVisitor visitor;
            return apply_visitor(visitor, mbglValue.stops);
        }
    };

    // Converts all types of mbgl property values that don't contain enumerations into an equivilant mgl style value
    class PropertyValueEvaluator {
    public:
        id operator()(const mbgl::style::Undefined) const {
            return nil;
        }

        id operator()(const MBGLType &value) const {
            auto rawValue = toMGLRawStyleValue(value);
            return [MGLStyleConstantValue<ObjCType> valueWithRawValue:rawValue];
        }

        id operator()(const mbgl::style::CameraFunction<MBGLType> &mbglValue) const {
            CameraFunctionStopsVisitor visitor;
            return apply_visitor(visitor, mbglValue.stops);
        }

        id operator()(const mbgl::style::SourceFunction<MBGLType> &mbglValue) const {
            SourceFunctionStopsVisitor visitor { mbglValue };
            return apply_visitor(visitor, mbglValue.stops);
        }

        // TODO: Implement me
        id operator()(const mbgl::style::CompositeFunction<MBGLType> &mbglValue) const {
            return nil;
        }
    };
};
