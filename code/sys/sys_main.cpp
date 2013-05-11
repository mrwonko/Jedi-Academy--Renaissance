#include <dlfcn.h>
#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/platform.h"
#include "../qcommon/files.h"

#include "sys_loadlib.h"
#include "sys_local.h"

static char cdPath[ MAX_OSPATH ] = { 0 };
static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
	Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
	return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
	Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
	if (*installPath)
		return installPath;
	else
		return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
	return Sys_BinaryPath();
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
	if( !dllHandle )
	{
		Com_Printf("Sys_UnloadDll(NULL)\n");
		return;
	}

	Sys_UnloadLibrary(dllHandle);
}

void Sys_SetDefaultCDPath(const char *path)
{
	Q_strncpyz(cdPath, path, sizeof(cdPath));
}

char *Sys_DefaultCDPath(void)
{
        return cdPath;
}

static void *game_library;

/*
 =================
 Sys_UnloadGame
 =================
 */
void Sys_UnloadGame (void)
{
	Com_Printf("------ Unloading Game ------\n");
	if (game_library)
		Sys_UnloadLibrary (game_library);
	game_library = NULL;
}

/*
 =================
 Sys_GetGameAPI
 
 Loads the game dll
 =================
 */
extern char		*FS_BuildOSPath( const char *base, const char *game, const char *qpath );

void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	
	char	name[MAX_OSPATH];
	char	*path;
	char	*basepath;
	char	*cdpath;
	char	*gamedir;
	char	*homepath;
	char	*fn;
	
	const char *gamename;
	if(Cvar_VariableIntegerValue("com_jk2"))
	{
		gamename = "jk2game" ARCH DLL_EXT;
	}
	else
	{
		gamename = "jagame" ARCH DLL_EXT;
	}
	
	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");
	
	// check the current debug directory first for development purposes
	homepath = Cvar_VariableString( "fs_homepath" );
	basepath = Cvar_VariableString( "fs_basepath" );
	cdpath = Cvar_VariableString( "fs_cdpath" );
	gamedir = Cvar_VariableString( "fs_game" );
	
	if(!gamedir || !gamedir[0])
		gamedir = BASEGAME;
	
	fn = FS_BuildOSPath( basepath, gamedir, gamename );
	
	game_library = Sys_LoadLibrary( fn );
	
//First try in mod directories. basepath -> homepath -> cdpath
	if (!game_library) {
		if (homepath[0]) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			
			fn = FS_BuildOSPath( homepath, gamedir, gamename);
			game_library = Sys_LoadLibrary( fn );
		}
	}
	
	if (!game_library) {
		if( cdpath[0] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			
			fn = FS_BuildOSPath( cdpath, gamedir, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
	
//Now try in base. basepath -> homepath -> cdpath
	if (!game_library) {
		Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		
		fn = FS_BuildOSPath( basepath, BASEGAME, gamename);
		game_library = Sys_LoadLibrary( fn );
	}
	
	if (!game_library) {
		if ( homepath[0] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			
			fn = FS_BuildOSPath( homepath, BASEGAME, gamename);
			game_library = Sys_LoadLibrary( fn );
		}
	}
	
	if (!game_library) {
		if( cdpath[0] ) {
			Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			
			fn = FS_BuildOSPath( cdpath, BASEGAME, gamename );
			game_library = Sys_LoadLibrary( fn );
		}
	}
//Still couldn't find it.
	if (!game_library) {
		Com_Printf( "Sys_GetGameAPI(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
		Com_Error( ERR_FATAL, "Couldn't load game" );
	}
	
	
	Com_Printf ( "Sys_GetGameAPI(%s): succeeded ...\n", fn );

	GetGameAPI = (void *(*)(void *))Sys_LoadFunction (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();
		return NULL;
	}
	
	return GetGameAPI (parms);
}

/*
 =================
 Sys_LoadCgame
 
 Used to hook up a development dll
 =================
 */
void * Sys_LoadCgame( int (**entryPoint)(int, ...), int (*systemcalls)(int, ...) )
{
	void	(*dllEntry)( int (*syscallptr)(int, ...) );
    
	dllEntry = ( void (*)( int (*)( int, ... ) ) )Sys_LoadFunction( game_library, "dllEntry" );
	*entryPoint = (int (*)(int,...))Sys_LoadFunction( game_library, "vmMain" );
	if ( !*entryPoint || !dllEntry ) {
		Sys_UnloadLibrary( game_library );
		return NULL;
	}
    
	dllEntry( systemcalls );
	return game_library;
}

int main (int argc, char **argv)
{
	int 	oldtime, newtime;
	int		len, i;
	char	*cmdline;
	void SetProgramPath(char *path);
	
	
	// get the initial time base
	Sys_Milliseconds();
	
	// merge the command line, this is kinda silly
	for (len = 1, i = 1; i < argc; i++)
		len += strlen(argv[i]) + 1;
	cmdline = (char *)malloc(len);
	*cmdline = 0;
	for (i = 1; i < argc; i++) {
		if (i > 1)
			strcat(cmdline, " ");
		strcat(cmdline, argv[i]);
	}
	Com_Init(cmdline);
		
    while (1)
    {
		// set low precision every frame, because some system calls
		// reset it arbitrarily
#ifdef _DEBUG
//		if (!g_wv.activeApp)
//		{
//			Sleep(50);
//		}
#endif // _DEBUG
        
        // make sure mouse and joystick are only called once a frame
		IN_Frame();
		
        Com_Frame ();
    }
}