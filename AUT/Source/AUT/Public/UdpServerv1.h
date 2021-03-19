// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class AUT_API UdpServerv1
{
	class UdpServer* us;
public:
	UdpServerv1(UINT16 port, int& iResultp, TFunction<void(char* data, UINT16& size)>receivework);
	~UdpServerv1();
	TFunction<void(char* data, UINT16& size)> onreceive;
	bool send(char* data, const UINT16& size);
	bool close();
};
