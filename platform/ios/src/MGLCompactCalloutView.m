#import "MGLCompactCalloutView.h"

#import "MGLAnnotation.h"

@implementation MGLCompactCalloutView
{
    id <MGLAnnotation> _representedObject;
}

@synthesize representedObject = _representedObject;
@synthesize automaticPositioning = _automaticPositioning;
@synthesize staysOpen = _staysOpen;

- (instancetype)init {
    if (self = [super init]) {
        _automaticPositioning = YES;
        _staysOpen = YES;
    }
    return self;
}

+ (instancetype)platformCalloutView
{
    return [[self alloc] init];
}

- (void)setRepresentedObject:(id <MGLAnnotation>)representedObject
{
    _representedObject = representedObject;
    
    if ([representedObject respondsToSelector:@selector(title)])
    {
        self.title = representedObject.title;
    }
    if ([representedObject respondsToSelector:@selector(subtitle)])
    {
        self.subtitle = representedObject.subtitle;
    }
}

@end
