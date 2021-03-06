// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperCharacter.h"
#include "AwesomeGameCharacter.generated.h"

// Create the Enum of States
UENUM(BlueprintType)
enum class EStateEnum :uint8{
		Idle,
		Running,
		Jumping,
		Shooting,
		Hit,
		Dead
};

// Create the structure used to attach States to Flipbooks
USTRUCT(BlueprintType)
struct AWESOMEGAME_API FStateFlipbookStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	EStateEnum State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	class UPaperFlipbook* Flipbook;
};

/**
 * Our main Character class
 * We setup the minimum required to extend it in Blueprint and finish it there.
 * THIS CLASS SHOULD NOT BE USED DIRECTLY HAS THE DEFAULTPAWNCLASS
 */
UCLASS(config=Game, Abstract)
class AWESOMEGAME_API AAwesomeGameCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

protected:
	/** EXPOSED PROPERTIES AND METHODS **/

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float DamageImpulseX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float DamageImpulseZ;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
	EStateEnum CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	TArray<FStateFlipbookStruct> StateFlipbookArray;

	// The following variables are self-explanatory and used for the State machine
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsJumping;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsRunning;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsHit;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsDead;

	// Updates the animation of the character
	UFUNCTION(BlueprintCallable, Category = Default)
	void UpdateAnimation();

	// StateMachine
	UFUNCTION(BlueprintCallable, Category = Default)
	EStateEnum StateMachine();

	// Returns the Flipbook associated to a State via the Struct
	UFUNCTION(BlueprintCallable, Category = Default)
	UPaperFlipbook* GetFlipbookForState(EStateEnum State);

	// Event called when the player dies
	UFUNCTION(BlueprintImplementableEvent, Category = Character)
	void OnDied();

	/** NON EXPOSED METHODS AND PROPERTIES **/

	// Represents the direction the character is facing, 1 = right, -1 = left
	int Facing;
	
	// Override TakeDamage event
	float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

	// Override the OnLanded event
	virtual void Landed(const FHitResult& Hit) override;

	// Deals with movement with MoveRight
	void MoveRight(float Value);

	// Deals with custom actions when with Jump then calls the Pawn method.
	void Jump();

	// Checks if the character is dead and deals with it.
	bool CheckDead();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface
public:
	AAwesomeGameCharacter();
	
	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
