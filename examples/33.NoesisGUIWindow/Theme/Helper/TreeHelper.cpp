#include "TreeHelper.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsGui/UIElement.h>
#include <NsGui/ContentControl.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>
#include <NsGui/Selector.h>
#include <NsGui/PasswordBox.h>
#include <NsGui/TextBox.h>
#include <NsApp/IrrNsGuiBindings.h>

using namespace WorldClient::Controls;
using namespace Noesis;

template<typename T>
T TreeHelper::TryFindParent(Visual* child)
{
	static_assert(std::is_base_of<Visual, T>::value, "T must inherit from Visual");

	//get parent item
	Visual* parentObject = GetParentObject(child);

	//we've reached the end of the tree
	if (parentObject == nullptr)
	{
		return nullptr;
	}

	//check if the parent matches the type we're looking for
	T parent = dynamic_cast<T*>(parentObject);
	return (parent != nullptr) ? parent : TryFindParent<T>(parentObject);
}

std::vector<Visual*> TreeHelper::GetAncestors(Visual * child)
{
	std::vector<Visual*> result;
	auto parent = VisualTreeHelper::GetParent(child);
	while (parent != nullptr)
	{
		result.push_back(parent);
		parent = VisualTreeHelper::GetParent(parent);
	}

	return std::move(result);
}

template<typename T>
T TreeHelper::FindChild(Visual * parent, const std::string & childName)
{
	static_assert(std::is_base_of<Visual, T>::value, "T must inherit from Visual");

	// Confirm parent and childName are valid. 
	if (parent == nullptr)
	{
		return nullptr;
	}

	T foundChild = nullptr;

	int childrenCount = VisualTreeHelper::GetChildrenCount(parent);
	for (int i = 0; i < childrenCount; i++)
	{
		auto child = VisualTreeHelper::GetChild(parent, i);
		// If the child is not of the request child type child
		T childType = dynamic_cast<T*>(child);
		if (childType == nullptr)
		{
			// recursively drill down the tree
			foundChild = FindChild<T>(child, childName);
			// If the child is found, break so we do not overwrite the found child. 
			if (foundChild != nullptr)
			{
				break;
			}
		}
		else if (!childName.empty())
		{
			auto frameworkInputElement = dynamic_cast<IFrameworkInputElement*>(child);
			// If the child's name is set for search
			if (frameworkInputElement != nullptr && frameworkInputElement->Name == childName)
			{
				// if the child's name is of the request name
				foundChild = static_cast<T>(child);
				break;
			}
			else
			{
				// recursively drill down the tree
				foundChild = FindChild<T>(child, childName);
				// If the child is found, break so we do not overwrite the found child. 
				if (foundChild != nullptr)
				{
					break;
				}
			}
		}
		else
		{
			// child element found.
			foundChild = static_cast<T>(child);
			break;
		}
	}

	return foundChild;
}

Visual* TreeHelper::GetParentObject(Visual * child)
{
	if (child == nullptr)
		return nullptr;

	// handle content elements separately
	//auto contentElement = NsDynamicCast<ContentElement*>(child);
	//if (contentElement != nullptr)
	//{
	//	Visual* parent = Noesis::ContentOperations::GetParent(contentElement);
	//	if (parent != nullptr)
	//	{
	//		return parent;
	//	}
	//
	//	auto fce = dynamic_cast<FrameworkElement*>(contentElement);
	//	return fce != nullptr ? fce->Parent : nullptr;
	//}

	auto childParent = VisualTreeHelper::GetParent(child);
	if (childParent != nullptr)
	{
		return childParent;
	}

	// also try searching for parent in framework elements (such as DockPanel, etc)
	auto frameworkElement = Noesis::DynamicCast<FrameworkElement*>(child);
	if (frameworkElement != nullptr)
	{
		Visual* parent = frameworkElement->GetParent();
		if (parent != nullptr)
		{
			return parent;
		}
	}

	return nullptr;
}

template<typename T>
std::vector<T> TreeHelper::FindChildren(Visual * source, bool forceUsingTheVisualTreeHelper)
{
	static_assert(std::is_base_of<Visual, T>::value, "T must inherit from Visual");

	std::vector<T> result;
	if (source != nullptr)
	{
		auto childs = GetChildObjects(source, forceUsingTheVisualTreeHelper);
		for (auto child : childs)
		{
			//analyze if children match the requested type
			if (child != nullptr && NsDynamicCast<T>(child) != nullptr)
			{
				result.push_back(NsStaticCast<T>(child));
			}

			//recurse tree
			for (T descendant : FindChildren<T>(child, forceUsingTheVisualTreeHelper))
			{
				result.push_back(descendant);
			}
		}
	}

	return std::move(result);
}

std::vector<Visual*> TreeHelper::GetChildObjects(Visual * parent, bool forceUsingTheVisualTreeHelper)
{
	std::vector<Visual*> result;
	if (parent == nullptr)
		return result;

	auto frameworkElement = Noesis::DynamicCast<FrameworkElement*>(parent);
	if (frameworkElement != nullptr)
	{
		//use the logical tree for content / framework elements
		size_t childCount = LogicalTreeHelper::GetChildrenCount(frameworkElement);
		for (size_t i = 0; i < childCount; ++i)
		{
			auto depObj = Noesis::DynamicPtrCast<Visual>(LogicalTreeHelper::GetChild(frameworkElement, i));
			if (depObj != nullptr)
				result.push_back(depObj);
		}
	}

	return result;
}

template<typename T>
T* TreeHelper::TryFindFromPoint(UIElement * reference, const Point& point)
{
	static_assert(std::is_base_of<Visual, T>::value, "T must inherit from Visual");

	HitTestResult hit = reference->HitTest(point);
	auto element =  NsDynamicCast<T*>(hit.visualHit);

	if (element)
		return element;
	return TryFindParent<T>(element);
}
