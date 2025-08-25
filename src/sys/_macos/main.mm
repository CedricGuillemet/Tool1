#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include "intro.h"

#ifdef TOOLS
#include "tools.h"
#include "imgui.h"
#include "imgui_impl_osx.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"
#endif

#include <stdio.h>
#include <stdarg.h>

void msys_debugPrintf(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    vprintf(format, arglist);
    //fflush(fp);
    va_end(arglist);
}

@interface AppView : NSOpenGLView
{
    NSTimer*    animationTimer;
}
@end

AppView* view;

@implementation AppView

-(void)prepareOpenGL
{
    [super prepareOpenGL];

#ifndef DEBUG
    GLint swapInterval = 1;
    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    if (swapInterval == 0)
        NSLog(@"Error: Cannot set swap interval.");
#endif
}

float dwidth, dheight, ddpi;

float GetDPIFactor()
{
    return ddpi;
}

-(void)initialize
{
    const float dpi = (float)[self.window backingScaleFactor];
    dwidth = (float)self.bounds.size.width * dpi;
    dheight = (float)self.bounds.size.height * dpi;
    ddpi = dpi;

    intro_init();

    [[self openGLContext] makeCurrentContext];
#ifdef TOOLS
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    tools_init();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplOSX_Init(self);
    ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
}

-(void)updateAndDrawDemoView
{
    glViewport(0, 0, dwidth, dheight);
    intro_do();
#ifdef TOOLS
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplOSX_NewFrame(self);
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // Start the Dear ImGui frame
    tools_do();
    
    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    [[self openGLContext] makeCurrentContext];
    GLsizei width  = (GLsizei)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    GLsizei height = (GLsizei)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    glViewport(0, 0, width, height);
    //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    //glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
#endif
    // Present
    [[self openGLContext] flushBuffer];

    if (!animationTimer)
        animationTimer = [NSTimer scheduledTimerWithTimeInterval:0.017 target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES];
}

-(void)reshape                              { [super reshape]; [[self openGLContext] update]; [self updateAndDrawDemoView]; }
-(void)drawRect:(NSRect)bounds              { [self updateAndDrawDemoView]; }
-(void)animationTimerFired:(NSTimer*)timer  { [self setNeedsDisplay:YES]; }
-(void)dealloc                              { animationTimer = nil; }

@end

void Present()
{
    [[view openGLContext] flushBuffer];
}

void MakeLoaderContextCurrent()
{
}

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, readonly) NSWindow* window;
@end

@implementation AppDelegate
@synthesize window = _window;

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

-(NSWindow*)window
{
    if (_window != nil)
        return (_window);

    NSRect viewRect = NSMakeRect(100.0, 100.0, 100.0 + 1280.0, 100 + 720.0);

    _window = [[NSWindow alloc] initWithContentRect:viewRect styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:YES];
    [_window setTitle:@"4Kb Demo"];
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setOpaque:YES];
    [_window orderFrontRegardless];

    return (_window);
}

-(void)setupMenu
{
    NSMenu* mainMenuBar = [[NSMenu alloc] init];
    NSMenu* appMenu;
    NSMenuItem* menuItem;

    appMenu = [[NSMenu alloc] initWithTitle:@"4Kb Demo"];
    menuItem = [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
    [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];

    menuItem = [[NSMenuItem alloc] init];
    [menuItem setSubmenu:appMenu];

    [mainMenuBar addItem:menuItem];

    appMenu = nil;
    [NSApp setMainMenu:mainMenuBar];
}

-(void)dealloc
{
    _window = nil;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Make the application a foreground application (else it won't receive keyboard events)
    ProcessSerialNumber psn = {0, kCurrentProcess};
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    // Menu
    [self setupMenu];

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core, // Core profile
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFADepthSize, 32,
            0
    };

    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    view = [[AppView alloc] initWithFrame:self.window.frame pixelFormat:format];
    format = nil;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        [view setWantsBestResolutionOpenGLSurface:YES];
#endif // MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    [self.window setContentView:view];

    if ([view openGLContext] == nil)
        NSLog(@"No OpenGL Context!");

    [view initialize];
}

@end

//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        NSApp = [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init];
        [[NSApplication sharedApplication] setDelegate:delegate];
        [NSApp run];
    }
    return NSApplicationMain(argc, argv);
}
