//
//  ViewController.mm
//  Application
//
//  Created by rechardchen on 12-4-25.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//
#import <QuartzCore/QuartzCore.h>
#import "ViewController.h"
#import "common.h"
#import "EAGLView.h"
#include "../EnvExt.hpp"
#include "../RuningExt.hpp"
#include "m3eTypes.h"

@interface ViewController ()

@property (retain, nonatomic) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

@synthesize context;
@synthesize displayLink;
@synthesize animationFrameInterval;
@synthesize animating;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];
    if (!context) 
    {
        NSLog(@"Failed to create ES context");
    }
    
    [(EAGLView *)self.view setContext: context];
    [(EAGLView *)self.view setFramebuffer];
    
    animating = FALSE;
    animationFrameInterval = 1;
    
    self.displayLink = nil;
    [self setupGL];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}


- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1) {
        animationFrameInterval = frameInterval;
        
        if (animating) {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

-(void)ImpLoop
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    env.runing().update();

    if (env.runing().rendEnable()) {
        [(EAGLView*)self.view setFramebuffer];
        env.runing().rend();
        [(EAGLView*)self.view presentFramebuffer];
    }
    
    id appDelegate = [[UIApplication sharedApplication] delegate];
    if ([appDelegate respondsToSelector:@selector(ShowFrame)])
    {
        [appDelegate performSelector:@selector(ShowFrame)];
    }
    
}
- (void)startAnimation
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
	
    if (!animating)
    {
        /*
         CADisplayLink is API new in iOS 3.1. Compiling against earlier versions will result in a warning, but can be dismissed if the system version runtime check for CADisplayLink exists in -awakeFromNib. The runtime check ensures this code will not be called in system versions earlier than 3.1.
         */
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(ImpLoop)];
        [displayLink setFrameInterval:animationFrameInterval];
        
        // The run loop will retain the display link on add.
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
    }
    
    [(EAGLView*)self.view setFramebuffer];
}

- (void)stopAnimation
{
    if (animating) {
        [self.displayLink invalidate];
        self.displayLink = nil;
        
        animating = FALSE;
    }
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if (interfaceOrientation == UIDeviceOrientationLandscapeRight || interfaceOrientation == UIDeviceOrientationLandscapeLeft)
    {
        return YES;
    }
	
	return NO;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());
    
    env.runing().update();
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());
    
    env.runing().rend();

}

-(void)touchedProcess:(NSSet*)touches withEvent:(UIEvent*)event
{
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    int curTouchID = -1;
    for(UITouch *myTouch in touches)
    {
        CGPoint newpoint = [myTouch locationInView:self.view];
        CGPoint oldpoint = [myTouch previousLocationInView:self.view];
        
        float scale = [[UIScreen mainScreen] scale];
        newpoint.x *= scale; newpoint.y *= scale;
        oldpoint.x *= scale; oldpoint.y *= scale;
        
        curTouchID = (ptr_int_t)myTouch;
        if(UITouchPhaseBegan == myTouch.phase)
        {
            env.runing().processInput(M3E_Touch_MOUSEBEGAN, curTouchID, newpoint.x, newpoint.y, oldpoint.x, oldpoint.y);
        }
        else if(UITouchPhaseMoved == myTouch.phase)
        {
            env.runing().processInput(M3E_Touch_MOUSEMOVED, curTouchID, newpoint.x, newpoint.y, oldpoint.x, oldpoint.y);
        }
        else if(UITouchPhaseEnded == myTouch.phase || UITouchPhaseCancelled == myTouch.phase)
        {
            env.runing().processInput(M3E_Touch_MOUSEENDED, curTouchID, newpoint.x, newpoint.y, oldpoint.x, oldpoint.y);
        }
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchedProcess:touches withEvent:event];
}

#pragma mark - Releasing

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
}

- (void)viewDidUnload
{    
    [super viewDidUnload];
    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) 
    {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (void)dealloc 
{
    [context release];
    [super dealloc];
}

@end


