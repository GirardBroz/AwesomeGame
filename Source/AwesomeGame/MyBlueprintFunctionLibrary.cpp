// Fill out your copyright notice in the Description page of Project Settings.

#include "AwesomeGame.h"
#include "MyBlueprintFunctionLibrary.h"

void UMyBlueprintFunctionLibrary::BlinkComponent(AActor* Actor, USceneComponent* Component, float BlinkDuration, float BlinkRate)
{
	// Create 2 timer handles for our 2 timers.
	FTimerHandle DoBlinkTimerHandle;
	FTimerHandle StopBlinkTimerHandle;

	// Since everything inside a BlueprintLibrary is Static, we have to use CreateStatic to get the Timer delegates working.
	FTimerDelegate DoBlinkDelegate = FTimerDelegate::CreateStatic(&UMyBlueprintFunctionLibrary::DoBlink, Component);
	// Set the DoBlink timer.
	Actor->GetWorldTimerManager().SetTimer(DoBlinkTimerHandle, DoBlinkDelegate, BlinkRate, true);
	
	// Set the second delegate and the attached Timer.
	// IMPORTANT: The Delegate takes the values as they are RIGHT NOW. If we create the delegate before using DoBlinkTimerHandle, it will be empty and it will never stop.
	FTimerDelegate StopBlinkDelegate = FTimerDelegate::CreateStatic(&UMyBlueprintFunctionLibrary::StopBlink, Actor, DoBlinkTimerHandle, Component);
	Actor->GetWorldTimerManager().SetTimer(StopBlinkTimerHandle, StopBlinkDelegate, BlinkDuration, false);
}

void UMyBlueprintFunctionLibrary::DoBlink(USceneComponent* Component)
{
	Component->ToggleVisibility();
}

void UMyBlueprintFunctionLibrary::StopBlink(AActor* Actor, FTimerHandle DoBlinkTimerHandle, USceneComponent* Component)
{
	Actor->GetWorldTimerManager().ClearTimer(DoBlinkTimerHandle);
	Component->SetVisibility(true);
}