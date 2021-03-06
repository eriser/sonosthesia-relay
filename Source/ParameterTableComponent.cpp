/*
  ==============================================================================

    ParameterTableComponent.cpp
    Created: 12 May 2016 4:37:39pm
    Author:  Jonathan Thorpe

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

#include "ParameterTableComponent.h"
#include "RelayComponent.h"

//==============================================================================
ParameterTableComponent::ParameterTableComponent(RelayAudioProcessor& _processor) : processor(_processor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    addAndMakeVisible(newButton);
    newButton.addListener(this);
    newButton.setButtonText("New");
    
    addAndMakeVisible(clearButton);
    clearButton.addListener(this);
    clearButton.setButtonText("Clear");
    
    // Create our table component and add it to this component..
    addAndMakeVisible (table);
    
    table.setModel (this);
    
    // give it a border
    table.setColour (ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness (1);
    
    TableHeaderComponent::ColumnPropertyFlags flags = (TableHeaderComponent::ColumnPropertyFlags)
    (TableHeaderComponent::ColumnPropertyFlags::visible |
     TableHeaderComponent::ColumnPropertyFlags::resizable);
    
    TableHeaderComponent::ColumnPropertyFlags buttonFlags = (TableHeaderComponent::ColumnPropertyFlags)
    (TableHeaderComponent::ColumnPropertyFlags::visible);
    
    table.getHeader().addColumn ("", buttonColumnId, 100, 50, 400, buttonFlags);
    table.getHeader().addColumn ("Parameter", parameterColumnId, 100, 50, 400, flags);
    table.getHeader().addColumn ("Target", targetColumnId, 150, 50, 400, flags);
    table.getHeader().addColumn ("Group", groupColumnId, 150, 50, 400, flags);
    table.getHeader().addColumn ("Descriptor", descriptorColumnId, 150, 50, 400, flags);
    
    table.setRowHeight(25);
    
    processor.getOSCTargetManager().addChangeListener(this);
    
}

ParameterTableComponent::~ParameterTableComponent()
{
}

void ParameterTableComponent::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (Colours::white);   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::lightblue);
    
    //g.setFont (14.0f);
    //g.drawText ("ParameterTableComponent", getLocalBounds(), Justification::centred, true);
}


void ParameterTableComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains...
    
    int buttonHeight = 25;
    int margin = 4;
    
    int halfWidth = getBounds().getWidth() / 2;
    
    newButton.setBounds( getBounds().withX(0).withY(0).withHeight(buttonHeight).withWidth(halfWidth).reduced(margin) );
    clearButton.setBounds( getBounds().withX(halfWidth).withY(0).withHeight(buttonHeight).withWidth(halfWidth).reduced(margin) );
    
    table.setBounds( getBounds().withX(0).withY(buttonHeight).withTrimmedBottom(buttonHeight).reduced(margin) );
}


void ParameterTableComponent::changeListenerCallback (ChangeBroadcaster *source)
{
    // if the targets change then we need to update the table so that the new targets are available
    if (source == dynamic_cast<ChangeBroadcaster*>(&processor.getOSCTargetManager()))
    {
        std::cout << "ParameterTableComponent updating table content on osc target manager change\n";
        table.updateContent();
    }
}

// ======== Button::Listener =========

void ParameterTableComponent::buttonClicked (Button* button)
{
    if (button == &newButton)
    {
        //std::cout << "Make new target!\n";
        processor.getParameterRelayManager().newItem();
        table.updateContent();
    }
    else if (button == &clearButton)
    {
        //std::cout << "Clear targets!\n";
        processor.getParameterRelayManager().clear();
        table.updateContent();
    }
}

// ======= TableListBoxModel =========

int ParameterTableComponent::getNumRows()
{
    return processor.getParameterRelayManager().count();
}

void ParameterTableComponent::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected) g.fillAll (Colours::lightblue);
    else if (rowNumber % 2) g.fillAll (Colour (0xffeeeeee));
}

String ParameterTableComponent::getCellText (const int columnId, const int rowNumber)
{
    std::shared_ptr<ParameterRelay> relay = processor.getParameterRelayManager().getItem(rowNumber);
    
    if (columnId == groupColumnId) return relay->getGroup();
    
    else if (columnId == descriptorColumnId) return relay->getDescriptor();
    
    throw std::invalid_argument("unexpected columnId");
}

void ParameterTableComponent::cellTextChanged (const int columnId, const int rowNumber, const String& newText)
{
    std::shared_ptr<ParameterRelay> relay = processor.getParameterRelayManager().getItem(rowNumber);
    
    if (columnId == groupColumnId) return relay->setGroup(String(newText));
    
    else if (columnId == descriptorColumnId) return relay->setDescriptor(String(newText));
    
    throw std::invalid_argument("unexpected columnId");
}

void ParameterTableComponent::onCellDeleteButton (const int rowNumber)
{
    std::cout << "Delete row " << rowNumber << "\n";
    
    std::shared_ptr<ParameterRelay> relay = processor.getParameterRelayManager().getItem(rowNumber);
    
    processor.getParameterRelayManager().deleteItem(relay->getIdentifier());
    
    table.updateContent();
}

void ParameterTableComponent::onCellDetailsButton (const int rowNumber)
{
    std::cout << "Details row " << rowNumber << "\n";
    
    //std::shared_ptr<MIDIRelay> relay = relayManager.getItems().at(rowNumber);
}

// This is overloaded from TableListBoxModel, and must update any custom components that we're using
Component* ParameterTableComponent::refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    
    if (columnId == buttonColumnId) // For the ratings column, we return the custom combobox component
    {
        ButtonCustomComponent* component = static_cast<ButtonCustomComponent*> (existingComponentToUpdate);
        
        // If an existing component is being passed-in for updating, we'll re-use it, but
        // if not, we'll have to create one.
        if (component == nullptr) component = new ButtonCustomComponent(*this);
        
        component->setRow(rowNumber);
        return component;
    }
    
    else if (columnId == parameterColumnId) // For the ratings column, we return the custom combobox component
    {
        ParameterCellComponent* component = static_cast<ParameterCellComponent*> (existingComponentToUpdate);
        
        // If an existing component is being passed-in for updating, we'll re-use it, but
        // if not, we'll have to create one.
        if (component == nullptr) component = new ParameterCellComponent(processor.getParameters());
        
        std::shared_ptr<ParameterRelay> relay = processor.getParameterRelayManager().getItem(rowNumber);
        
        component->setRelay(relay);
        return component;
    }
    
    else if (columnId == targetColumnId) // For the ratings column, we return the custom combobox component
    {
        TargetSelectionComponent* component = static_cast<TargetSelectionComponent*> (existingComponentToUpdate);
        
        // If an existing component is being passed-in for updating, we'll re-use it, but
        // if not, we'll have to create one.
        if (component == nullptr) component = new TargetSelectionComponent(processor.getOSCTargetManager());
        
        std::shared_ptr<Relay> relay = std::static_pointer_cast<Relay>( processor.getParameterRelayManager().getItem(rowNumber) );
        
        component->refresh();
        component->setRelay(relay);
        
        return component;
    }
    
    else if (columnId == groupColumnId || columnId == descriptorColumnId)
    {
        // The other columns are editable text columns, for which we use the custom Label component
        TextCellComponent* textLabel = static_cast<TextCellComponent*> (existingComponentToUpdate);
        
        // same as above...
        if (textLabel == nullptr)
            textLabel = new TextCellComponent (*this);
        
        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }
    
    return nullptr;
    
}


// This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom components.
void ParameterTableComponent::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour (Colours::black);
}

int ParameterTableComponent::getColumnAutoSizeWidth (int columnId)
{
    return 32;
}


