// Fill out your copyright notice in the Description page of Project Settings.


#include "UdpServerv1.h"
#include "UdpServer.h"

UdpServerv1::UdpServerv1(UINT16 port, int& iResultp, TFunction<void(char* data, UINT16& size)>receivework)
{
	us = new UdpServer(port, iResultp, receivework);
}

UdpServerv1::~UdpServerv1()
{
	us->close();
	delete us;
}
bool UdpServerv1::send(char* data, const UINT16& size)
{
	return us->send(data, size);
}
bool UdpServerv1::close()
{
  return us->close();
}
