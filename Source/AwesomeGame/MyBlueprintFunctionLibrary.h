// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AWESOMEGAME_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Makes a component blink every BlinkRate time for a desired amount of time.
	 *
	 * @param Actor The Actor to attach the timer to, usually the Actor that posses the component.
	 * @param Component The component to blink.
	 * @param BlinkDuration How long should we blink ? Defaults to 2seconds.
	 * @param BlinkRate How fast should we blink ? Defaults to 0.1seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "MyBlueprintFunctionLibrary")
	static void BlinkComponent(AActor* Actor, USceneComponent* Component, float BlinkDuration = 2.0f, float BlinKRate = 0.1f);

	/**
	 * Does the actual switching of the visibility of the component.
	 *
	 * @param Component The component to blink.
	 */
	static void DoBlink(USceneComponent* Component);

	/**
	* Stops the DoBlinkTimer and sets the visibility back to true.
	*
	* @param Actor The Actor to whom the Handle is attached to.
	* @param DoBlinkTimerHandle The DoBlinkTimerHandle to stop.
	* @param Component The component to switch back to visibility true.
	*/
	static void StopBlink(AActor* Actor, FTimerHandle DoBlinkTimerHandle, USceneComponent* Component);
};
