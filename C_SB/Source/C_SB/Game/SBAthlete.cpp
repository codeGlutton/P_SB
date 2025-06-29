// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SBAthlete.h"

// Sets default values
ASBAthlete::ASBAthlete()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASBAthlete::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASBAthlete::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASBAthlete::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

