#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#import "MGLFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/**
 An `MGLStyleValue` object is a generic container for a style attribute value.
 The layout and paint attribute properties of `MGLStyleLayer` can be set to
 `MGLStyleValue` objects.

 The `MGLStyleValue` class itself represents a class cluster. Under the hood, a
 particular `MGLStyleValue` object may be either an `MGLStyleConstantValue` to
 represent a constant value or an `MGLStyleFunction` to represent a value
 function. Do not initialize an `MGLStyleValue` object directly; instead, use
 one of the class factory methods to create an `MGLStyleValue` object.
 
 The `MGLStyleValue` class takes a generic parameter `T` that indicates the
 Foundation class being wrapped by this class. Common values for `T` include:
 
 <ul>
 <li>`NSNumber` (for Boolean values, floating-point numbers, and enumerations)</li>
 <li>`NSValue` (for `CGVector`, `NSEdgeInset`, and `UIEdgeInset`s)</li>
 <li>`NSString`</li>
 <li>`NSColor` or `UIColor`</li>
 <li>`NSArray`</li>
 </ul>
 */
MGL_EXPORT
@interface MGLStyleValue<T> : NSObject

#pragma mark Creating a Style Value

/**
 Creates and returns an `MGLStyleConstantValue` object containing a raw value.
 
 @param rawValue The constant value contained by the object.
 @return An `MGLStyleConstantValue` object containing `rawValue`, which is
    treated as a constant value.
 */
+ (instancetype)valueWithRawValue:(T)rawValue;

#pragma mark Camera function values

/**
 Creates and returns an `MGLStyleFunction` object representing a linear zoom
 level function with any number of stops.
 
 @param stops A dictionary associating zoom levels with style values.
 @return An `MGLStyleFunction` object with the given stops.
 */
+ (instancetype)valueWithStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)stops;

// TODO: deprecate in favor of the same API but with "exponentialStops"
/**
 Creates and returns an `MGLStyleFunction` object representing a zoom level
 function with an exponential base and any number of stops.
 
 @param base The exponential base of the interpolation curve.
 @param stops A dictionary associating zoom levels with style values.
 @return An `MGLStyleFunction` object with the given base and stops.
 */
+ (instancetype)valueWithBase:(CGFloat)base stops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)stops;

// TODO: API docs
+ (instancetype)valueWithIntervalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)intervalStops;

#pragma mark Source function values

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary<id, MGLStyleValue<T> *> *)categoricalStops defaultValue:(MGLStyleValue<T> * _Nullable)defaultValue;

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName exponentialStops:(NSDictionary<id, MGLStyleValue<T> *> *)exponentialStops;

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary<id, MGLStyleValue<T> *> *)exponentialStops;

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary<id, MGLStyleValue<T> *> *)intervalStops;

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName;

// TODO: API docs
+ (instancetype)valueWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)categoricalStops;

@end

/**
 An `MGLStyleConstantValue` object is a generic container for a style attribute
 value that remains constant as the zoom level changes. The layout and paint
 attribute properties of `MGLStyleLayer` objects can be set to
 `MGLStyleConstantValue` objects.
 
 The `MGLStyleConstantValue` class takes a generic parameter `T` that indicates
 the Foundation class being wrapped by this class.
 */
MGL_EXPORT
@interface MGLStyleConstantValue<T> : MGLStyleValue<T>

#pragma mark Creating a Style Constant Value

/**
 Creates and returns an `MGLStyleConstantValue` object containing a raw value.
 
 @param rawValue The constant value contained by the object.
 @return An `MGLStyleConstantValue` object containing `rawValue`, which is
    treated as a constant value.
 */
+ (instancetype)valueWithRawValue:(T)rawValue;

#pragma mark Initializing a Style Constant Value

- (instancetype)init NS_UNAVAILABLE;

/**
 Returns an `MGLStyleConstantValue` object containing a raw value.
 
 @param rawValue The value contained by the receiver.
 @return An `MGLStyleConstantValue` object containing `rawValue`.
 */
- (instancetype)initWithRawValue:(T)rawValue NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Underlying Value

/**
 The raw value contained by the receiver.
 */
@property (nonatomic) T rawValue;

@end

/**
 An `MGLStyleFunction` is a value function defining a style value that changes
 as the zoom level changes. The layout and paint attribute properties of an
 `MGLStyleLayer` object can be set to `MGLStyleFunction` objects. Use a zoom
 level function to create the illusion of depth and control data density.
 
 The `MGLStyleFunction` class takes a generic parameter `T` that indicates the
 Foundation class being wrapped by this class.
 */
MGL_EXPORT
@interface MGLStyleFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Function

/**
 Creates and returns an `MGLStyleFunction` object representing a linear zoom
 level function with any number of stops.
 
 @param stops A dictionary associating zoom levels with style values.
 @return An `MGLStyleFunction` object with the given stops.
 */
+ (instancetype)functionWithStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)stops;

/**
 Creates and returns an `MGLStyleFunction` object representing a zoom level
 function with an exponential base and any number of stops.
 
 @param base The exponential base of the interpolation curve.
 @param stops A dictionary associating zoom levels with style values.
 @return An `MGLStyleFunction` object with the given base and stops.
 */
+ (instancetype)functionWithBase:(CGFloat)base stops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)stops;

#pragma mark Initializing a Style Function

/**
 Returns an `MGLStyleFunction` object representing a zoom level function with an
 exponential base and any number of stops.
 
 @param base The exponential base of the interpolation curve.
 @param stops A dictionary associating zoom levels with style values.
 @return An `MGLStyleFunction` object with the given base and stops.
 */
- (instancetype)initWithBase:(CGFloat)base stops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)stops NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Function

/**
 The exponential base of the function’s interpolation curve.
 
 The exponential base controls the rate at which the function’s output values
 increase. A value of 1 causes the function to increase linearly by zoom level.
 A higher exponential base causes the function’s output values to vary
 exponentially, increasing more rapidly towards the high end of the function’s
 range. The default value of this property is 1, for a linear curve.
 */
@property (nonatomic) CGFloat base;

/**
 A dictionary associating zoom levels with style values.
 
 Each of the function’s stops is represented by one key-value pair in the
 dictionary. Each key in the dictionary is an `NSNumber` object containing a
 floating-point zoom level. Each value in the dictionary is an `MGLStyleValue`
 object containing the value of the style attribute when the map is at the
 associated zoom level. An `MGLStyleFunction` object may not be used recursively
 as a stop value.
 */
@property (nonatomic, copy) NSDictionary<NSNumber *, MGLStyleValue<T> *> *stops;

@end

/**
 An `MGLStyleIntervalFunction` is a value function defining a style value that 
 changes as the zoom level changes over a discrete domain. The layout and paint 
 attribute properties of an `MGLStyleLayer` object can be set to 
 `MGLStyleIntervalFunction` objects. Use a zoom level function to create the 
 illusion of depth and control data density.

 The `MGLStyleIntervalFunction` class takes a generic parameter `T` that indicates the
 Foundation class being wrapped by this class.
 */
@interface MGLStyleIntervalFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Interval Function

// TODO: API docs
+ (instancetype)functionWithIntervalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)intervalStops;

#pragma mark Initializing a Style Interval Function

// TODO: API docs
- (instancetype)initWithIntervalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)intervalStops NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Interval Function

// TODO: API docs
@property (nonatomic, copy) NSDictionary<NSNumber *, MGLStyleValue<T> *> *stops;

@end

//todo: break MGLStyleSourceFunction into SourceCategorical, SourceExponential, SourceInterval, and SourceIdentity

// TODO: API docs
@interface MGLStyleSourceCategoricalFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Source Categorical Function

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary<id, MGLStyleValue<T> *> *)categoricalStops defaultValue:(MGLStyleValue<T> * _Nullable)defaultValue;

#pragma mark Initializing a Style Source Categorical Function

// TODO: API docs
- (instancetype)initWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary<id, MGLStyleValue<T> *> *)categoricalStops defaultValue:(MGLStyleValue<T> * _Nullable)defaultValue NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Source Categorical Function

// TODO: API docs
@property (nonatomic, copy) NSString *attributeName;

// TODO: API docs
@property (nonatomic, copy) NSDictionary<id, MGLStyleValue<T> *> *stops;

// TODO: API docs
@property (nonatomic, nullable) MGLStyleValue<T> *defaultValue;

@end

// TODO: API docs
@interface MGLStyleSourceExponentialFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Source Exponential Function

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName exponentialStops:(NSDictionary<id, MGLStyleValue<T> *> *)exponentialStops;

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary<id, MGLStyleValue<T> *> *)exponentialStops;

#pragma mark Initializing a Style Source Exponential Function

// TODO: API docs
- (instancetype)initWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary<id, MGLStyleValue<T> *> *)exponentialStops NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Source Exponential Function

// TODO: API docs
@property (nonatomic, copy) NSString *attributeName;

// TODO: API docs
@property (nonatomic) CGFloat base;

// TODO: API docs
@property (nonatomic, copy) NSDictionary<NSNumber *, MGLStyleValue<T> *> *stops;

@end

// TODO: API docs
@interface MGLStyleSourceIntervalFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Source Interval Function

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary<id, MGLStyleValue<T> *> *)intervalStops;

#pragma mark Initializing a Style Source Interval Function

// TODO: API docs
- (instancetype)initWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary<id, MGLStyleValue<T> *> *)intervalStops NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Source Interval Function

// TODO: API docs
@property (nonatomic, copy) NSString *attributeName;

// TODO: API docs
@property (nonatomic, copy) NSDictionary<NSNumber *, MGLStyleValue<T> *> *stops;

@end

// TODO: API docs
@interface MGLStyleSourceIdentityFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Source Identity Function

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName;

#pragma mark Initializing a Style Source Identity Function

// TODO: API docs
- (instancetype)initWithAttributeName:(NSString *)attributeName NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Source Identity Function

// TODO: API docs
@property (nonatomic, copy) NSString *attributeName;

@end

// TODO: API docs
@interface MGLStyleCompositeCategoricalFunction<T> : MGLStyleValue<T>

#pragma mark Creating a Style Composite Categorical Function

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)categoricalStops;

#pragma mark Initializing a Style Composite Categorical Function

// TODO: API docs
- (instancetype)initWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary<NSNumber *, MGLStyleValue<T> *> *)categoricalStops NS_DESIGNATED_INITIALIZER;

#pragma mark Accessing the Parameters of a Style Composite Categorical Function

// TODO: API docs
@property (nonatomic, copy) NSString *attributeName;

// TODO: API docs
@property (nonatomic, copy) NSDictionary<NSNumber *, MGLStyleValue<T> *> *stops;

@end

NS_ASSUME_NONNULL_END
