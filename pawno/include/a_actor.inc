#if defined _actor_included
	#endinput
#endif
#define _actor_included

native CreateActor(modelid, Float:X, Float:Y, Float:Z, Float:Rotation);
native DestroyActor(actorid);
native IsValidActor(actorid);