// Fill out your copyright notice in the Description page of Project Settings.

#include "GlowBrushStructCustomization.h"
#include "DetailWidgetRow.h"
#include "GlowImage.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"

bool FGlowBrushIdentifier::IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const
{
	TArray<UObject*> OuterObjects;
	PropertyHandle.GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 0)
	{
		return false;
	}

	for (const UObject* Outer : OuterObjects)
	{
		if (!IsValid(Outer) || !Outer->IsA(UGlowImage::StaticClass()))
		{
			return false;
		}
	}

	return true;
}

TSharedRef<IPropertyTypeCustomization> FGlowBrushStructCustomization::MakeInstance()
{
	return MakeShareable(new FGlowBrushStructCustomization());
}

void FGlowBrushStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];
}

void FGlowBrushStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	static const FName ShownChildren[] = {
		FName(TEXT("ResourceObject")),
	};

	for (const FName& ChildName : ShownChildren)
	{
		if (TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildName))
		{
			StructBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
}
