// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SBAthlete.generated.h"

UCLASS()
class C_SB_API ASBAthlete : public ACharacter
{
	GENERATED_BODY()

public:
	ASBAthlete();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
