#import "MGLStyleValue_Private.h"

@implementation MGLStyleValue

+ (instancetype)valueWithRawValue:(id)rawValue {
    return [MGLStyleConstantValue valueWithRawValue:rawValue];
}

+ (instancetype)valueWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    return [MGLStyleFunction functionWithBase:base stops:stops];
}

+ (instancetype)valueWithStops:(NSDictionary *)stops {
    return [MGLStyleFunction functionWithStops:stops];
}

+ (instancetype)valueWithIntervalStops:(NSDictionary *)intervalStops {
    return [MGLStyleIntervalFunction functionWithIntervalStops:intervalStops];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary *)categoricalStops defaultValue:(id)defaultValue {
    return [MGLStyleSourceCategoricalFunction functionWithAttributeName:attributeName categoricalStops:categoricalStops defaultValue:defaultValue];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName exponentialStops:(NSDictionary *)exponentialStops {
    return [MGLStyleSourceExponentialFunction functionWithAttributeName:attributeName exponentialStops:exponentialStops];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary *)exponentialStops {
    return [MGLStyleSourceExponentialFunction functionWithAttributeName:attributeName base:base exponentialStops:exponentialStops];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary *)intervalStops {
    return [MGLStyleSourceIntervalFunction functionWithAttributeName:attributeName intervalStops:intervalStops];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName {
    return [MGLStyleSourceIdentityFunction functionWithAttributeName:attributeName];
}

+ (instancetype)valueWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary *)categoricalStops {
    return [MGLStyleCompositeCategoricalFunction functionWithAttributeName:attributeName compositeCategoricalStops:categoricalStops];
}

@end

@implementation MGLStyleConstantValue

+ (instancetype)valueWithRawValue:(id)rawValue {
    return [[self alloc] initWithRawValue:rawValue];
}

- (instancetype)initWithRawValue:(id)rawValue {
    if (self = [super init]) {
        _rawValue = rawValue;
    }
    return self;
}

- (NSString *)description {
    return [self.rawValue description];
}

- (NSString *)debugDescription {
    return [self.rawValue debugDescription];
}

- (BOOL)isEqual:(MGLStyleConstantValue *)other {
    return [other isKindOfClass:[self class]] && [other.rawValue isEqual:self.rawValue];
}

- (NSUInteger)hash {
    return [self.rawValue hash];
}

@end

@implementation MGLStyleFunction

+ (instancetype)functionWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    return [[self alloc] initWithBase:base stops:stops];
}

+ (instancetype)functionWithStops:(NSDictionary *)stops {
    return [[self alloc] initWithBase:1 stops:stops];
}

- (instancetype)init {
    return [self initWithBase:1 stops:@{}];
}

- (instancetype)initWithBase:(CGFloat)base stops:(NSDictionary *)stops {
    if (self = [super init]) {
        if (!stops.count)
        {
            [NSException raise:NSInvalidArgumentException format:@"%@ requires at least one stop.", self];
        }
        _base = base;
        _stops = stops;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, base = %f; stops = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.base,
            self.stops];
}

- (BOOL)isEqual:(MGLStyleFunction *)other {
    return ([other isKindOfClass:[self class]] && other.base == self.base
            && [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash {
    return self.base + self.stops.hash;
}

@end

@implementation MGLStyleIntervalFunction

+ (instancetype)functionWithIntervalStops:(NSDictionary *)stops {
    return [[self alloc] initWithIntervalStops:stops];
}

- (instancetype)init {
    return [self initWithIntervalStops:@{}];
}

- (instancetype)initWithIntervalStops:(NSDictionary *)stops {
    if (self == [super init]) {
        _stops = stops;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, stops = %@>", NSStringFromClass([self class]), (void *)self, self.stops];
}

- (BOOL)isEqual:(MGLStyleIntervalFunction *)other {
    return ([other isKindOfClass:[self class]] && [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash {
    return self.stops.hash;
}

@end

@implementation MGLStyleSourceCategoricalFunction

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary *)categoricalStops defaultValue:(id)defaultValue {
    return [[self alloc] initWithAttributeName:attributeName categoricalStops:categoricalStops defaultValue:defaultValue];
}

- (instancetype)init {
    return [self initWithAttributeName:@"" categoricalStops:@{} defaultValue:nil];
}

- (instancetype)initWithAttributeName:(NSString *)attributeName categoricalStops:(NSDictionary *)categoricalStops defaultValue:(id)defaultValue {
    if (self == [super init]) {
        _attributeName = attributeName;
        _stops = categoricalStops;
        _defaultValue = defaultValue;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, attributeName = %@, \
                                                 categoricalStops = %@ \
                                                 defaultValue = %@>",
            NSStringFromClass([self class]),
            (void *)self,
            self.attributeName,
            self.stops,
            self.defaultValue];
}

- (BOOL)isEqual:(MGLStyleSourceCategoricalFunction *)other {
    return ([other isKindOfClass:[self class]] &&
            [other.attributeName isEqual:self.attributeName] &&
            [other.stops isEqualToDictionary:self.stops] &&
            [other.defaultValue isEqual:self.defaultValue]);
}

- (NSUInteger)hash {
    return self.attributeName.hash + self.stops.hash + self.defaultValue.hash;
}

@end

@implementation MGLStyleSourceExponentialFunction

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName exponentialStops:(NSDictionary *)exponentialStops {
    return [[self alloc] initWithAttributeName:attributeName base:1.0 exponentialStops:exponentialStops];
}

// TODO: API docs
+ (instancetype)functionWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary *)exponentialStops {
    return [[self alloc] initWithAttributeName:attributeName base:base exponentialStops:exponentialStops];
}

- (instancetype)init {
    return [self initWithAttributeName:@"" base:0 exponentialStops:@{}];
}

- (instancetype)initWithAttributeName:(NSString *)attributeName base:(CGFloat)base exponentialStops:(NSDictionary *)exponentialStops {
    if (self == [super init]) {
        _attributeName = attributeName;
        _base = base;
        _stops = exponentialStops;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, attributeName = %@, \
                                                 stops = %@ \
                                                 base = %f>",
            NSStringFromClass([self class]),
            (void *)self,
            self.attributeName,
            self.stops,
            self.base];
}

- (BOOL)isEqual:(MGLStyleSourceExponentialFunction *)other {
    return ([other isKindOfClass:[self class]] &&
            [other.attributeName isEqual:self.attributeName] &&
            [other.stops isEqualToDictionary:self.stops] &&
            other.base == self.base);
}

- (NSUInteger)hash {
    return self.attributeName.hash + self.stops.hash + self.base;
}

@end

@implementation MGLStyleSourceIntervalFunction

+ (instancetype)functionWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary *)intervalStops {
    return [[self alloc] initWithAttributeName:attributeName intervalStops:intervalStops];
}

- (instancetype)init {
    return [self initWithAttributeName:@"" intervalStops:@{}];
}

- (instancetype)initWithAttributeName:(NSString *)attributeName intervalStops:(NSDictionary *)intervalStops {
    if (self == [super init]) {
        _attributeName = attributeName;
        _stops = intervalStops;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, attributeName = %@, \
            stops = %@>",
            NSStringFromClass([self class]),
            (void *)self,
            self.attributeName,
            self.stops];
}

- (BOOL)isEqual:(MGLStyleSourceExponentialFunction *)other {
    return ([other isKindOfClass:[self class]] &&
            [other.attributeName isEqual:self.attributeName] &&
            [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash {
    return self.attributeName.hash + self.stops.hash;
}

@end

@implementation MGLStyleSourceIdentityFunction

+ (instancetype)functionWithAttributeName:(NSString *)attributeName {
    return [[self alloc] initWithAttributeName:attributeName];
}

- (instancetype)init {
    return [self initWithAttributeName:@""];
}

- (instancetype)initWithAttributeName:(NSString *)attributeName {
    if (self == [super init]) {
        _attributeName = attributeName;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, attributeName = %@>",
            NSStringFromClass([self class]),
            (void *)self,
            self.attributeName];
}

- (BOOL)isEqual:(MGLStyleSourceExponentialFunction *)other {
    return ([other isKindOfClass:[self class]] &&
            [other.attributeName isEqual:self.attributeName]);
}

- (NSUInteger)hash {
    return self.attributeName.hash;
}

@end

@implementation MGLStyleCompositeCategoricalFunction

+ (instancetype)functionWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary<NSNumber *, NSDictionary *> *)categoricalStops {
    return [[self alloc] initWithAttributeName:attributeName compositeCategoricalStops:categoricalStops];
}

- (instancetype)init {
    return [self initWithAttributeName:@"" compositeCategoricalStops:@{}];
}

- (instancetype)initWithAttributeName:(NSString *)attributeName compositeCategoricalStops:(NSDictionary *)categoricalStops {
    if (self == [super init]) {
        _attributeName = attributeName;
        _stops = categoricalStops;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@: %p, attributeName = %@, \
            stops = %@>",
            NSStringFromClass([self class]),
            (void *)self,
            self.attributeName,
            self.stops];
}

- (BOOL)isEqual:(MGLStyleSourceExponentialFunction *)other {
    return ([other isKindOfClass:[self class]] &&
            [other.attributeName isEqual:self.attributeName] &&
            [other.stops isEqualToDictionary:self.stops]);
}

- (NSUInteger)hash {
    return self.attributeName.hash + self.stops.hash;
}

@end
