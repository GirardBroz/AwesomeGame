// Fill out your copyright notice in the Description page of Project Settings.

#include "AwesomeGame.h"
#include "AwesomeGameCharacter.h"
#include "PaperFlipbookComponent.h"

////////////////////////////
// Our main Character class
// We setup the minimum required to extend it in Blueprint and finish it there.
// THIS CLASS SHOULD NOT BE USED DIRECTLY HAS THE DEFAULTPAWNCLASS

AAwesomeGameCharacter::AAwesomeGameCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bAbsoluteRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 1600.0f;
	SideViewCameraComponent->AttachTo(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->bAbsoluteRotation = true;
	SideViewCameraComponent->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;	

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	// Set the default states
	isJumping = false;
	isRunning = false;
	isDead = false;

	// Set full life
	currentHealth = 1.0f;

	// Some default values for the knockback when the character takes damage
	DamageImpulseX = 20000.0f;
	DamageImpulseZ = 20000.0f;
}

void AAwesomeGameCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	InputComponent->BindAxis("MoveRight", this, &AAwesomeGameCharacter::MoveRight);
	
	InputComponent->BindAction("Jump", IE_Pressed, this, &AAwesomeGameCharacter::Jump);
}

void AAwesomeGameCharacter::MoveRight(float Value)
{	
	// Turns the character depending on if we are going Right or Left.
	if (Controller != nullptr)
	{
		if (Value > 0)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
			isRunning = true;
			Facing = -1;
		}
		else if (Value < 0)
		{
			Controller->SetControlRotation(FRotator(0.0f, 180.0f, 0.0f));
			isRunning = true;
			Facing = 1;
		}
		else
		{
			isRunning = false;
		}
	}

	// Add the input
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);

	StateMachine();
	UpdateAnimation();
}

void AAwesomeGameCharacter::Jump()
{
	Super::Jump();
	isJumping = true;
}

void AAwesomeGameCharacter::Landed(const FHitResult& Hit){
	Super::Landed(Hit);
	isJumping = false;

	// The player landed from a damage-induced knockback, he's not hurt anymore
	if (isHit)
	{
		isHit = false;
	}
}

void AAwesomeGameCharacter::UpdateAnimation()
{
	GetSprite()->SetFlipbook(GetFlipbookForState(CurrentState));
}

UPaperFlipbook* AAwesomeGameCharacter::GetFlipbookForState(EStateEnum State)
{
	FStateFlipbookStruct LoopState;
	for (int32 b = 0; b < StateFlipbookArray.Num(); b++)
	{
		LoopState = StateFlipbookArray[b];
		if (LoopState.State == State)
		{
			return LoopState.Flipbook;
		}
	}
	return false;
}

EStateEnum AAwesomeGameCharacter::StateMachine()
{
	if (isHit)
	{
		CurrentState = EStateEnum::Hit;
	}
	else if (isJumping)
	{
		CurrentState = EStateEnum::Jumping;
	}
	else if (isRunning)
	{
		CurrentState = EStateEnum::Running;
	}
	else {
		CurrentState = EStateEnum::Idle;
	}

	return CurrentState;
}

float AAwesomeGameCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	// Call the base class - this will tell us how much damage to apply  
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		currentHealth -= ActualDamage;
		// Check if the character is dead and deal with it if needed
		CheckDead();

		GetCharacterMovement()->AddImpulse(FVector(Facing*DamageImpulseX, 0.0f, DamageImpulseZ));

		isHit = true;
	}

	return ActualDamage;
}

bool AAwesomeGameCharacter::CheckDead()
{
	if (currentHealth <= 0.0f)
	{
		isDead = true;
		DisableInput((APlayerController*)Controller);
	}
	else {
		isDead = false;
	}

	return isDead;
}