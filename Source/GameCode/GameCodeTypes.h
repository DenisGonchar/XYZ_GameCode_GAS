#pragma once

#define ECC_Climbing ECC_GameTraceChannel1
#define ECC_InteractionVolume ECC_GameTraceChannel2
#define ECC_Bullet ECC_GameTraceChannel3
#define ECC_Melee ECC_GameTraceChannel4
#define ECC_WallRannable ECC_GameTraceChannel5
#define ECC_RockClimbing ECC_GameTraceChannel6

const FName SocketFPCamera = FName("CameraSocket");
const FName SocketCharacterWeapon = FName("CharacterWeaponSocket");
const FName SocketCharacterThrowable = FName("ThrowableSocket");
const FName SocketWeaponMuzzle = FName("MuzzleSocket");
const FName SocketWeaponForeGrip = FName("ForeGripSocket");

const FName CollisionProfilePawn = FName("Pawn");
const FName CollisionProfileRagdoll = FName("Ragdoll");
const FName CollisionProfilePawnInteractionVolume = FName("PawnInteractionVolume");
const FName CollosionProfileNoCollision = FName("NoCollision");

const FName DebugCategoryLedgeDetection = FName("LedgeDetection");
const FName DebugCategoryCharacterAttributes = FName("CharacterAttributes");
const FName DebugCategoryWallRun = FName("WallRun");
const FName DebugCategoryRangeWeapon = FName("RangeWeapon");
const FName DebugCategoryMeleeWeapon = FName("MeleeWeapon");

const FName FXParamTraceEnd = FName("TraceEnd");
const FName SectionMontageReloadEnd = FName("ReloadEnd");

const FName BB_CurrentTarget = FName("CurrentTarget");
const FName BB_NextLocation = FName("NextLocation");

const FName ActionInteract = FName("Interact");

const FName SignificanceTagCharacter = FName("Character");

const float SignificanceValueVeryHith = 0.0f;
const float SignificanceValueHith = 1.0f;
const float SignificanceValueMedium = 10.0f;
const float SignificanceValueLow = 100.0f;
const float SignificanceValueVeryLow = 1000.0f;

UENUM(BlueprintType)
enum class EEquipableItemType : uint8
{
	None,
	Pistol, 
	Rifle,
	Throwable,
	Melee,
	Bow
	
};

UENUM(BlueprintType)
enum class EAmmunitionType : uint8
{
	None,
	Pistol,
	Rifle,
	ShotgunShells,
	FragGrenades,
	RifleGrenades,
	Arrow,
	MAX UMETA(Hidden)

};

UENUM(BlueprintType)
enum class EEquipmentSlots : uint8
{
	None,
	SideArm,
	PrimaryWeapon,
	SecondaryWeapon,
	PrimaryItemSlot,
	MeleeWeapon,
	MAX UMETA(Hidden)

};

UENUM(BlueprintType)
enum class EReticleType : uint8
{
	None,
	Default,
	SniperRifle,
	Bow,
	MAX UMETA(Hidden)

};

UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	None,
	PrimeryAttack,
	SecondaryAttack,
	MAX UMETA(Hidden)
	
};

UENUM(BlueprintType)
enum class ETeams : uint8
{
	Player,
	Enemy
};

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Head,
	Body,
	LHand,
	RHand,
	LLeg,
	RLeg

};

UENUM(BlueprintType)
enum class ETypesCharacter : uint8
{
	Default,
	Modular
};