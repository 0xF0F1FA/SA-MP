#if defined _actor_included
	#endinput
#endif
#define _actor_included

native CreateActor(modelid, Float:X, Float:Y, Float:Z, Float:Rotation);
native DestroyActor(actorid);
native SetActorVirtualWorld(actorid, vworld);
native GetActorFacingAngle(actorid, &Float:ang);
native GetActorHealth(actorid, &Float:health);
native SetActorInvulnerable(actorid, invulnerable = true);
native IsActorInvulnerable(actorid);
native IsValidActor(actorid);