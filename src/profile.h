#pragma once

#define ENUM_ELEMENT(a)	a,
#define BUILD_ENUM(a)	enum { a(ENUM_ELEMENT) };
#define STRING_VERSION(a)	#a,
#define BUILD_STRINGS(a)	{ a(STRING_VERSION) };
#define	INC(a)				+1
#define COUNT(a)	(0 a(INC))

// PROFILE_ENABLE: disabled (0), enabled (1), master (2)
#ifdef ANDROID
#define PROFILE_ENABLE 0
#else
#define PROFILE_ENABLE 1
#endif

#if PROFILE_ENABLE
//#define MAX_PROFILES 8
#define	profiles(f) \
		f(PROFILE_MAIN) \
		f(PROFILE_ENGINE_TICK) \
		f(PROFILE_ENGINE_RENDER) \
		f(PROFILE_ENGINE_WAIT) \
		f(PROFILE_RENDER_TICK)

#define kMaxProfiles COUNT(profiles)

BUILD_ENUM(profiles)
#if PROFILE_ENABLE == 1
#define PROFILE_BEGIN(a)		profileBegin(a);
#define PROFILE_END(a)			profileEnd(a);
#define PROFILE_COMMAND(a)		a
#else
#define PROFILE_BEGIN(a)		/* disabled profile */
#define PROFILE_END(a)			/* disabled profile */
#define PROFILE_COMMAND(a)		/* disabled profile */
#endif // PROFILE_ENABLE==1
#define PROFILE_MASTER_BEGIN(a)	profileBegin(a);
#define PROFILE_MASTER_END(a)	profileEnd(a);
#else // PROFILE_ENABLE
#define PROFILE_BEGIN(a)		/* disabled profile */
#define PROFILE_END(a)			/* disabled profile */
#define PROFILE_COMMAND(a)		/* disabled profile */

#define PROFILE_MASTER_BEGIN(a)	/* disabled profile */
#define PROFILE_MASTER_END(a)	/* disabled profile */
#endif // PROFILE_ENABLE

#if PROFILE_ENABLE
void profile_newFrame();
void profileBegin(int index);
void profileEnd(int index);
void profile_init();
void profile_dumpProfileTimes();
#endif
