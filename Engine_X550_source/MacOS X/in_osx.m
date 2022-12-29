//_________________________________________________________________________________________________________________________nFO
// "in_osx.m" - MacOS X mouse driver
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
// Quakeª is copyrighted by id software		[http://www.idsoftware.com].
//
// Version History:
// v1.0.8: F12 eject is now disabled while Quake is running.
// v1.0.6: Mouse in_sensitivity works now as expected.
// v1.0.5: Reworked whole mouse handling code [required for windowed mouse].
// v1.0.0: Initial release.
//____________________________________________________________________________________________________________________iNCLUDES

#pragma mark =Includes=

#import <AppKit/AppKit.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/hidsystem/IOHIDLib.h>
#import <IOKit/hidsystem/IOHIDParameter.h>

#ifdef __i386__
#import <IOKit/hidsystem/event_status_driver.h>
#else
#import <drivers/event_status_driver.h>
#endif // __i386__

#import "quakedef.h"
#import "in_osx.h"
#import "vid_osx.h"

#pragma mark -

//_____________________________________________________________________________________________________________________sTATICS

#pragma mark =Variables=

cvar_t				aux_look = {"auxlook","1", true};
cvar_t				m_filter = {"m_filter","1"};
BOOL				gInMouseEnabled;
UInt8				gInSpecialKey[] = {
										K_UPARROW,    K_DOWNARROW,   K_LEFTARROW,    K_RIGHTARROW,
											 K_F1,           K_F2,          K_F3,            K_F4,
											 K_F5,           K_F6,          K_F7,            K_F8,
											 K_F9,          K_F10,         K_F11,           K_F12,
											K_PRINTSCR,          K_SCRLCK,         K_PAUSE,				0,
												0,				0,				0,				0,
												0,				0,				0,				0,
												0,				0,				0,				0,
												0,				0,				0,				0,
												0,				0,				0,          K_INS,
											K_DEL,		   K_HOME,				0,			K_END,
										   K_PGUP,         K_PGDN,	            0,				0,
										  K_PAUSE,				0,	            0,				0,
												0,				0,	            0,				0,
												0, 	    KP_NUMLOCK,				0,				0,                                                                        					    0, 	  	     0, 	     0, 	     0,
												0,				0,				0,				0,
												0,				0,			K_INS,				0
                                  };
UInt8					gInNumPadKey[] =  {	
												0,				0,	            0,              0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,				0,	            0,				0,
												0,		KP_DEL,	            0,		  KP_STAR,
												0,	   KP_PLUS,	            0,				0,
												0,				0,	            0,				0,
									  KP_ENTER,    KP_SLASH,    KP_MINUS,				0,
												0,    K_OSX_EQUAL_PAD,        KP_INS,        KP_END,
										  KP_DOWNARROW,        KP_PGDN,        KP_LEFTARROW,        KP_5,
										  K_RIGHTARROW,        KP_HOME,              0,        KP_UPARROW,
										  KP_PGUP,				0,	            0,				0
                                 };

static BOOL					gInMouseMoved;
static in_mousepos_t		gInMousePosition,
							gInMouseNewPosition,
							gInMouseOldPosition;

#pragma mark -

//_________________________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

static io_connect_t	IN_GetIOHandle (void);
static void 		IN_SetMouseScalingEnabled (BOOL theState);

#pragma mark -

//__________________________________________________________________________________________________________Toggle_AuxLook_f()

void	Toggle_AuxLook_f (void)
{
    if (aux_look.floater)
    {
        Cvar_SetFloatByRef (&aux_look, 0);
    }
    else
    {
        Cvar_SetFloatByRef (&aux_look, 1);
    }
}

//________________________________________________________________________________________________________Force_CenterView_f()

void	Force_CenterView_f (void)
{
    cl.viewangles[PITCH] = 0; 
}

//_______________________________________________________________________________________________IN_SetKeyboardRepeatEnabled()

void	IN_SetKeyboardRepeatEnabled (BOOL theState)
{
    static BOOL		myKeyboardRepeatEnabled = YES;
    static double	myOriginalKeyboardRepeatInterval;
    static double	myOriginalKeyboardRepeatThreshold;
    NXEventHandle	myEventStatus;
    
    if (theState == myKeyboardRepeatEnabled)
        return;
    if (!(myEventStatus = NXOpenEventStatus ()))
        return;
        
    if (theState)
    {
        NXSetKeyRepeatInterval (myEventStatus, myOriginalKeyboardRepeatInterval);
        NXSetKeyRepeatThreshold (myEventStatus, myOriginalKeyboardRepeatThreshold);
        NXResetKeyboard (myEventStatus);
    }
    else
    {
        myOriginalKeyboardRepeatInterval = NXKeyRepeatInterval (myEventStatus);
        myOriginalKeyboardRepeatThreshold = NXKeyRepeatThreshold (myEventStatus);
        NXSetKeyRepeatInterval (myEventStatus, 3456000.0f);
        NXSetKeyRepeatThreshold (myEventStatus, 3456000.0f);
    }
    
    NXCloseEventStatus (myEventStatus);
    myKeyboardRepeatEnabled = theState;
}

//____________________________________________________________________________________________________________IN_GetIOHandle()

io_connect_t IN_GetIOHandle (void)
{
    io_connect_t 	myHandle = MACH_PORT_NULL;
    kern_return_t	myStatus;
    io_service_t	myService = MACH_PORT_NULL;
    mach_port_t		myMasterPort;

    myStatus = IOMasterPort (MACH_PORT_NULL, &myMasterPort );
	
    if (myStatus != KERN_SUCCESS)
    {
        return (0);
    }

    myService = IORegistryEntryFromPath (myMasterPort, kIOServicePlane ":/IOResources/IOHIDSystem");

    if (myService == 0)
    {
        return (0);
    }

    myStatus = IOServiceOpen (myService, mach_task_self (), kIOHIDParamConnectType, &myHandle);
    IOObjectRelease (myService);

    return (myHandle);
}

//_____________________________________________________________________________________________________IN_SetF12EjectEnabled()

void	IN_SetF12EjectEnabled (qbool theState)
{
    static BOOL		myF12KeyIsEnabled = YES;
    static UInt32	myOldValue;
    io_connect_t	myIOHandle = 0;
    
    // Do we have a state change?
    if (theState == myF12KeyIsEnabled)
    {
        return;
    }

    // Get the IOKit handle:
    myIOHandle = IN_GetIOHandle ();
	
    if (myIOHandle == 0)
    {
        return;
    }

    // Set the F12 key according to the current state:
    if (theState == NO && keybindings[K_F12] != NULL && keybindings[K_F12][0] != 0x00)
    {
        UInt32			myValue = 0x00;
        IOByteCount		myCount;
        kern_return_t	myStatus;
        
        myStatus = IOHIDGetParameter (myIOHandle,
                                      CFSTR (kIOHIDF12EjectDelayKey),
                                      sizeof (UInt32),
                                      &myOldValue,
                                      &myCount);

        // change only the settings, if we were successfull!
        if (myStatus != kIOReturnSuccess)
        {
            theState = YES;
        }
        else
        {
            IOHIDSetParameter (myIOHandle, CFSTR (kIOHIDF12EjectDelayKey), &myValue, sizeof (UInt32));
        }
    }
    else
    {
        if (myF12KeyIsEnabled == NO)
        {
            IOHIDSetParameter (myIOHandle, CFSTR (kIOHIDF12EjectDelayKey),  &myOldValue, sizeof (UInt32));
        }
        theState = YES;
    }
    
    myF12KeyIsEnabled = theState;
    IOServiceClose (myIOHandle);
}

//_________________________________________________________________________________________________IN_SetMouseScalingEnabled()

void	IN_SetMouseScalingEnabled (BOOL theState)
{
	return;
#if 0	// Baker: Why does everything have to screw with your mouse, sheesh!
    static BOOL		myMouseScalingEnabled	= YES;
    static double	myOldAcceleration		= 0.0;
    io_connect_t	myIOHandle				= 0;

    // Do we have a state change?
    if (theState == myMouseScalingEnabled)
    {
        return;
    }
    
    // Get the IOKit handle:
    myIOHandle = IN_GetIOHandle ();
	
    if (myIOHandle == 0)
    {
        return;
    }

    // Set the mouse acceleration according to the current state:
    if (theState == YES)
    {
        IOHIDSetAccelerationWithKey (myIOHandle,
                                     CFSTR (kIOHIDMouseAccelerationType),
                                     myOldAcceleration);
    }
    else
    {
        kern_return_t	myStatus;

        myStatus = IOHIDGetAccelerationWithKey (myIOHandle,
                                                CFSTR (kIOHIDMouseAccelerationType),
                                                &myOldAcceleration);

        // change only the settings, if we were successfull!
        if (myStatus != kIOReturnSuccess || myOldAcceleration == 0.0)
        {
            theState = YES;
        }
         
        // finally change the acceleration:
        if (theState == NO)
        {
            IOHIDSetAccelerationWithKey (myIOHandle,  CFSTR (kIOHIDMouseAccelerationType), -1.0);
        }
    }
    
    myMouseScalingEnabled = theState;
    IOServiceClose (myIOHandle);
#endif
}

//_____________________________________________________________________________________________________________IN_ShowCursor()

void	IN_ShowCursor (BOOL theState)
{
    static BOOL		myCursorIsVisible = YES;

    // change only if we got a state change:
    if (theState != myCursorIsVisible)
    {
        if (theState == YES)
        {
            CGAssociateMouseAndMouseCursorPosition (YES);
            IN_SetMouseScalingEnabled (YES);
            IN_CenterCursor ();
            CGDisplayShowCursor (kCGDirectMainDisplay);
        }
        else
        {
            [NSApp activateIgnoringOtherApps: YES];
            CGDisplayHideCursor (kCGDirectMainDisplay);
            CGAssociateMouseAndMouseCursorPosition (NO);
            IN_CenterCursor ();
            IN_SetMouseScalingEnabled (NO);
        }
        myCursorIsVisible = theState;
    }
}

//___________________________________________________________________________________________________________IN_CenterCursor()

void	IN_CenterCursor (void)
{
    CGPoint		myCenter;

    if (gVidDisplayFullscreen == NO)
    {
        float		myCenterX = gVidWindowPosX, myCenterY = -gVidWindowPosY;

        // calculate the window center:
        myCenterX += (float) (vid.width >> 1);
        myCenterY += (float) CGDisplayPixelsHigh (kCGDirectMainDisplay) - (float) (vid.height >> 1);
        
        myCenter = CGPointMake (myCenterX, myCenterY);
    }
    else
    {
        // just center at the middle of the screen:
        myCenter = CGPointMake ((float) (vid.width >> 1), (float) (vid.height >> 1));
    }

    // and go:
    CGDisplayMoveCursorToPoint (kCGDirectMainDisplay, myCenter);
}

//______________________________________________________________________________________________________________IN_InitMouse()

void	IN_InitMouse (void)
{
    // check for command line:
    if (COM_CheckParm ("-nomouse"))
    {
        gInMouseEnabled = NO;
        return;
    }
    else
    {
        gInMouseEnabled = YES;
    }
    
    gInMouseMoved = NO;
}

//___________________________________________________________________________________________________________________IN_Init()

void 	IN_Init (void)
{
//    extern cvar_t m_accel;
    // register variables:
    Cvar_RegisterVariable (&m_filter, NULL);
    Cvar_RegisterVariable (&in_accel, NULL);
    Cvar_RegisterVariable (&aux_look, NULL);
    
    // register console commands:
    Cmd_AddCommand ("toggle_auxlook", Toggle_AuxLook_f);
    Cmd_AddCommand ("force_centerview", Force_CenterView_f);
    
    // init the mouse:
    IN_InitMouse ();
    
    IN_SetMouseScalingEnabled (NO);
    
	
    // enable mouse look by default:
    // Baker: voided this, we are using freelook cvar
    // Cbuf_AddText ("+mlook\n");
}

//_______________________________________________________________________________________________________________IN_Shutdown()

void 	IN_Shutdown (void)
{
    IN_SetMouseScalingEnabled (YES);
}

//_______________________________________________________________________________________________________________IN_Commands()

void 	IN_Mouse_Commands_OSX (void)
{
    // avoid popping of the app back to the front:
    if ([NSApp isHidden] == YES)
    {
        return;
    }
    
    // set the cursor visibility by respecting the display mode:
    if (gVidDisplayFullscreen == 1.0f)
    {
        IN_ShowCursor (NO);
    }
    else
    {
        // is the mouse in windowed mode?
        if (gInMouseEnabled == YES && [NSApp isActive] == YES &&
            Minimized == NO && (_windowed_mouse.floater != 0.0f && key_dest == key_game))
        {
            IN_ShowCursor (NO);
        }
        else
        {
            IN_ShowCursor (YES);
        }
    }
}

//_______________________________________________________________________________________________________IN_ReceiveMouseMove()

void	IN_ReceiveMouseMove (CGMouseDelta theDeltaX, CGMouseDelta theDeltaY)
{
    gInMouseNewPosition.X += theDeltaX;
    gInMouseNewPosition.Y += theDeltaY;
}

//______________________________________________________________________________________________________________IN_MouseMove()


void	IN_MouseMove (usercmd_t *cmd)
{
    CGMouseDelta	myMouseX = gInMouseNewPosition.X,
					myMouseY = gInMouseNewPosition.Y;

    if ((gVidDisplayFullscreen == NO && !_windowed_mouse.integer) ||
        gInMouseEnabled == NO || Minimized == YES || [NSApp isActive] == NO)
    {
        return;
    }

#if SUPPORTS_XFLIP
	if(scene_invert_x.integer) myMouseX *= -1;   //Atomizer - GL_XFLIP
#endif

    gInMouseNewPosition.X = 0;
    gInMouseNewPosition.Y = 0;

    if (in_filter.floater)
    {
        float filterfrac = CLAMP (0, in_filter.floater, 1) / 2.0;
        gInMousePosition.X = (myMouseX * (1 - filterfrac) + gInMouseOldPosition.X * filterfrac);
        gInMousePosition.Y = (myMouseY * (1 - filterfrac) + gInMouseOldPosition.Y * filterfrac);
    }
    else
    {
        gInMousePosition.X = myMouseX;
        gInMousePosition.Y = myMouseY;
    }

    gInMouseOldPosition.X = myMouseX;
    gInMouseOldPosition.Y = myMouseY;

	// Baker: Scale the sensitivity so that FOV 90 is the norm, everything is relative to FOV 90
	//        There are 360 degrees of possible view.  Seeing 180 degrees is half.  90 degrees 1/4.
	//        Right now, we are going to scale both X and Y sensitivity by FOV X.
	//        The reason for this is that we don't want different aiming "feels" for different
	//        screen resolutions.  And since we use corrective FOV rendering, we cannot count on 
	//        the refdef screen fov_x to be the same as cvar FOV (or even predictable).
	//        So we are going to use the cvar FOV for now.  And if we add fancier ways to control
	//        weapon zooming and such that doesn't use the CVAR, we'll go from there.
	do
	{
		float my_sensitivity = in_sensitivity.floater;
		float effective_sensitivity_x = my_sensitivity;
		float effective_sensitivity_y = my_sensitivity;

		if (in_fovscale.integer && scene_fov_x.floater /* fov shouldn't ever be zero, but anyways .. */)
		{
			float fov_x_factor = scene_fov_x.floater / 90;
			effective_sensitivity_x *= fov_x_factor;
			effective_sensitivity_y *= fov_x_factor; // Not a typo, multiplying both by fov_x_factor
		}


		if (in_accel.floater) 
		{
			float mx =  gInMousePosition.X -  gInMouseOldPosition.X;
			float my =  gInMousePosition.Y -  gInMouseOldPosition.Y;
			float mousespeed = sqrt (mx * mx + my * my);
			gInMousePosition.X *= (mousespeed * in_accel.floater + effective_sensitivity_x);
			gInMousePosition.Y *= (mousespeed * in_accel.floater + effective_sensitivity_y);
		}
		else 
		{
			gInMousePosition.X *= effective_sensitivity_x;
			gInMousePosition.Y *= effective_sensitivity_y;
		}
	} while (0);

    // lookstrafe or view?
    if ((in_strafe.state & 1) || (in_lookstrafe.integer && mlook_active))
        cmd->sidemove += mouse_lookstrafe_speed_side.floater * gInMousePosition.X;
    else
        cl.viewangles[YAW] -= mouse_speed_yaw.floater * gInMousePosition.X;
                
    if (mlook_active)
        View_StopPitchDrift ();
            
    if (mlook_active && !(in_strafe.state & 1))
    {
        float pitchchange = mouse_speed_pitch.floater * gInMousePosition.Y; 
        
		// Baker: new invert mouse scheme
		if (in_invert_pitch.integer)
			pitchchange = -pitchchange;

		cl.viewangles[PITCH] += pitchchange;
		CL_BoundViewPitch (cl.viewangles);	// mouse lock..  Covers directinput too.

	}
	else
	{
		if ((in_strafe.state & 1) && cl.noclip_anglehack)
            cmd->upmove -= mouse_lookstrafe_speed_forward.floater * gInMousePosition.Y;
        else
            cmd->forwardmove -= mouse_lookstrafe_speed_forward.floater * gInMousePosition.Y;
    }

    // if the mouse has moved, force it to the center, so there's room to move
    if (myMouseX || myMouseY)
    {
        IN_CenterCursor ();
    }
}

//___________________________________________________________________________________________________________________IN_Move()

void IN_Move (usercmd_t *cmd)
{
    IN_MouseMove (cmd);
}

//_________________________________________________________________________________________________________________________eOF
