////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __WorldClient_MAINWINDOW_H__
#define __WorldClient_MAINWINDOW_H__


#include <NsCore/Noesis.h>
#include <NsApp/Window.h>


namespace WorldClient
{

////////////////////////////////////////////////////////////////////////////////////////////////////
class MainWindow final: public NoesisApp::Window
{
public:
    MainWindow();

private:
    void InitializeComponent();
    void OnInitialized(Noesis::BaseComponent*, const Noesis::EventArgs&);

private:
    NS_DECLARE_REFLECTION(MainWindow, Window)
};


}


#endif
