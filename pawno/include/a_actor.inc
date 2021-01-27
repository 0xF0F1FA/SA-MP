/*  SA-MP Actor Functions
 *
 *  (c) Copyright 2015, SA-MP Team
 *
 */

#if defined _actor_included
	#endinput
#endif
#define _actor_included

native CreateActor(modelid, Float:X, Float:Y, Float:Z, Float:Rotation);
native DestroyActor(actorid);

native IsActorStreamedIn(actorid, forplayerid);

native SetActorVirtualWorld(actorid, vworld);
native GetActorVirtualWorld(actorid);
native SetActorPos(actorid, Float:X, Float:Y, Float:Z);
native GetActorPos(actorid, &Float:X, &Float:Y, &Float:Z);
native GetActorFacingAngle(actorid, &Float:ang);
native GetActorHealth(actorid, &Float:health);
native SetActorInvulnerable(actorid, invulnerable = true);
native IsActorInvulnerable(actorid);
native IsValidActor(actorid);