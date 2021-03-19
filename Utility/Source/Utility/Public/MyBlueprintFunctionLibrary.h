// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Core/Public/Containers/Array.h"
#include "Runtime/CoreUObject/Public/UObject/Object.h"
#include "MyBlueprintFunctionLibrary.generated.h"
DECLARE_DELEGATE(Fonge);
DECLARE_DELEGATE_TwoParams(Fang, const TArray<uint8>&, const FString &);
/**
 * 
 */
UCLASS()
class UTILITY_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static TArray<FString> MountedPakList;
public:
	///UTF8 is not stable often not work ,so jus use utf16
	//!!!!!!!!!!!FStringtoUTF8 this blueprint function ofent enconter some strange thing
	static void FStringtoUTF8(FString &in, uint8* &out, int32& outsize);
	static FString UTF8toFString(const TArray<uint8>&in);
	static void FStringtoUTF16(FString &in, uint8* &out, int64& outsize);
	static FString UTF16toFString(const TArray<uint8>&in, int64 size);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void readstringfromfile(FString filepath, FString & content);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void readdatafromfile(FString filepath, TArray<uint8> & content);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static bool FileMd5isequalSpecificMd5(FString filepath, FString SpecificMd5);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void writedatatofile(FString filepath, const TArray<uint8> & content);
	static void writedatatofile(FString &filepath,  uint8*& ByteBuffer,  int64& size);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static bool Mount(FString PakFileName);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static bool UnMount(FString PakFileName);
	static void RawImageToTexture2D(const TArray<uint8> &data,class UTexture2D *& out_texture);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void CLogtofile(FString msg);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static UObject* Loadobject(FString path);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static const FString Screenshoot(FString infilename,bool bInShowUI,bool bAddFilenameSuffix, bool brelativepath = true);
	UFUNCTION(BlueprintCallable, Category = "Mybp")
		static TArray<FString> getcamerainfor();
	UFUNCTION(BlueprintCallable, Category = "Mybp")
		static FLinearColor getcolorinforundercursor();
	UFUNCTION(BlueprintCallable, Category = "Mybp")
		static TArray<FString> findallfileunderpath(FString path, FString FileExtension);
	static void* CallBlueprintProperty(const UObject* otherobj, const FName propertyName);
	static void CallBlueprintfunction(const UObject* otherobj, const FName functionName, void* para);
	template <class T>
	static void sortarrray(TArray<T*>& arr, TFunction<bool(T* A, T* B)> compare)
	{
		for (int i = 0; i < arr.Num() - 1; i++)
		{
			T* flag = (T*)arr[i];
			int ti = i;
			for (int j = i + 1; j < arr.Num(); j++)
			{
				if (compare(arr[j], flag))
				{
					flag = (T*)arr[j];
					ti = j;
				}
			}
			arr[ti] = arr[i];
			arr[i] = flag;
		}
	}
	template <class T>
	static void sortarrray(TArray<T>& arr, TFunction<bool(const T &A, const T& B)> compare)
	{
		for (int i = 0; i < arr.Num() - 1; i++)
		{
			T flag = (T)arr[i];
			int ti = i;
			for (int j = i + 1; j < arr.Num(); j++)
			{
				if (compare(arr[j], flag))
				{
					flag = (T)arr[j];
					ti = j;
				}
			}
			arr[ti] = arr[i];
			arr[i] = flag;
		}
	}
public:
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void Ongameinitfunc();
	UFUNCTION(BlueprintCallable, Category = "Mybp")
	static void OnGameEndefunc();
	static TArray<Fonge> OnGameexit;
	static TArray<Fonge> OnGameInitevent;
	static TArray<Fang> OnGameexiteventwithparameter;
	static void AddfunctiontoOnGameIniteventArray(TBaseStaticDelegateInstance<TTypeWrapper<void>()>::FFuncPtr InFuncPtr);
	static void AddfunctiontoOnGameexitArray(TBaseStaticDelegateInstance<TTypeWrapper<void>()>::FFuncPtr InFuncPtr);
	static void AddfunctiontoOnGameIniteventwithparameterArray(TBaseStaticDelegateInstance<TTypeWrapper<void>(const TArray<uint8>&, const FString &)>::FFuncPtr InFuncPtr);
};
