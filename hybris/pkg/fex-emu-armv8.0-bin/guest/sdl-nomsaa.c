/*
 * sdl-nomsaa: preload for x86 SDL2 games under FEX on the phone.
 *
 * The OpenGL here is Mesa's llvmpipe through Xwayland, which has no
 * multisampled GLX visuals, and none flagged "accelerated". Games that insist
 * on them (Half-Life 25th anniversary asks for 4x MSAA even with -nomsaa)
 * fail with "Couldn't find matching GLX visual". This drops those requests.
 * Built as 32-bit x86: gcc -m32 -shared -fPIC -o sdl-nomsaa.so sdl-nomsaa.c -ldl
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>

enum {
	SDL_GL_MULTISAMPLEBUFFERS = 13,
	SDL_GL_MULTISAMPLESAMPLES = 14,
	SDL_GL_ACCELERATED_VISUAL = 15,
};

int SDL_GL_SetAttribute(int attr, int value)
{
	static int (*real)(int, int);
	void *h;

	if (!real) {
		real = (int (*)(int, int))dlsym(RTLD_NEXT, "SDL_GL_SetAttribute");
		if (!real && (h = dlopen("libSDL2-2.0.so.0", RTLD_NOW)))
			real = (int (*)(int, int))dlsym(h, "SDL_GL_SetAttribute");
		if (!real)
			return -1;
	}
	switch (attr) {
	case SDL_GL_MULTISAMPLEBUFFERS:
	case SDL_GL_MULTISAMPLESAMPLES:
		value = 0;
		break;
	case SDL_GL_ACCELERATED_VISUAL:
		value = -1;	/* SDL's "don't care" */
		break;
	}
	return real(attr, value);
}
