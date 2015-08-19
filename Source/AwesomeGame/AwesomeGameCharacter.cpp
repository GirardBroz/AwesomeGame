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
	SideViewCameraComponent->OrthoWidth = 2000.0f;
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

	// Tweak character movement according to our needs
	GetCharacterMovement()->GravityScale = 3.0f;
	GetCharacterMovement()->MaxAcceleration = 5000.0f;
	GetCharacterMovement()->SetWalkableFloorAngle(45.0f);
	GetCharacterMovement()->JumpZVelocity = 1200.0f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1000.0f;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	// Set the default states
	bIsJumping = false;
	bIsRunning = false;
	bIsDead = false;

	// Set full life
	CurrentHealth = 1.0f;

	// Some default values for the knockback when the character takes damage
	/*DamageImpulseX = 20000.0f;
	DamageImpulseZ = 20000.0f;*/
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
			bIsRunning = true;
			Facing = -1;
		}
		else if (Value < 0)
		{
			Controller->SetControlRotation(FRotator(0.0f, 180.0f, 0.0f));
			bIsRunning = true;
			Facing = 1;
		}
		else
		{
			bIsRunning = false;
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
	bIsJumping = true;
}

void AAwesomeGameCharacter::Landed(const FHitResult& Hit){
	Super::Landed(Hit);
	bIsJumping = false;

	// The player landed from a damage-induced knockback, he's not hurt anymore
	if (bIsHit)
	{
		bIsHit = false;
	}
}

void AAwesomeGameCharacter::UpdateAnimation()
{
	GetSprite()->SetFlipbook(GetFlipbookForState(CurrentState));
}

UPaperFlipbook* AAwesomeGameCharacter::GetFlipbookForState(EStateEnum State)
{
	for (auto& LoopState : StateFlipbookArray)
	{
		if (LoopState.State == State)
		{
			return LoopState.Flipbook;
		}
	}
	return false;
}

EStateEnum AAwesomeGameCharacter::StateMachine()
{
	if (bIsDead)
	{
		CurrentState = EStateEnum::Dead;
	}
	else if (bIsHit)
	{
		CurrentState = EStateEnum::Hit;
	}
	else if (bIsJumping)
	{
		CurrentState = EStateEnum::Jumping;
	}
	else if (bIsRunning)
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
		CurrentHealth -= ActualDamage;
		// Check if the character is dead and deal with it if needed
		CheckDead();

		// Impulse the character back to mark that he took damage
		//GetCharacterMovement()->AddImpulse(FVector(Facing*DamageImpulseX, 0.0f, DamageImpulseZ));

		bIsHit = true;
	}

	return ActualDamage;
}

bool AAwesomeGameCharacter::CheckDead()
{
	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		DisableInput((APlayerController*)Controller);

		// Call the Blueprint event OnDied in case we want some fancy stuff to happen in BP when the char dies
		OnDied();
	}
	else {
		bIsDead = false;
	}

	return bIsDead;
}