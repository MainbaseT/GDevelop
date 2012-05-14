/** \file
 *  Game Develop
 *  2008-2012 Florian Rival (Florian.Rival@gmail.com)
 */

#if defined(GD_IDE_ONLY)

#include "WhileEvent.h"
#include "GDL/tinyxml/tinyxml.h"
#include "GDL/RuntimeScene.h"
#include "GDL/OpenSaveGame.h"
#include "GDL/Events/EventsCodeGenerator.h"
#include "GDL/Events/ExpressionsCodeGeneration.h"
#include "GDCore/IDE/EventsRenderingHelper.h"
#include "GDCore/IDE/EventsEditorItemsAreas.h"
#include "GDCore/IDE/EventsEditorSelection.h"
#include "PlatformDefinition/InstructionsMetadataHolder.h"
#include "GDL/Events/EventsCodeGenerationContext.h"
#include "GDL/ExtensionsManager.h"

std::string WhileEvent::GenerateEventCode(Game & game, Scene & scene, EventsCodeGenerator & codeGenerator, EventsCodeGenerationContext & parentContext)
{
    std::string outputCode;

    //Context is "reset" each time the event is repeated ( i.e. objects are picked again )
    EventsCodeGenerationContext context;
    context.InheritsFrom(parentContext);

    //Prepare codes
    std::string whileConditionsStr = codeGenerator.GenerateConditionsListCode(game, scene, whileConditions, context);
    std::string whileIfPredicat = "true"; for (unsigned int i = 0;i<whileConditions.size();++i) whileIfPredicat += " && condition"+ToString(i)+"IsTrue";
    std::string conditionsCode = codeGenerator.GenerateConditionsListCode(game, scene, conditions, context);
    std::string actionsCode = codeGenerator.GenerateActionsListCode(game, scene, actions, context);
    std::string ifPredicat = "true"; for (unsigned int i = 0;i<conditions.size();++i) ifPredicat += " && condition"+ToString(i)+"IsTrue";

    //Write final code
    outputCode += "bool stopDoWhile = false;";
    outputCode += "do";
    outputCode += "{\n";
    outputCode += context.GenerateObjectsDeclarationCode();
    outputCode +=  whileConditionsStr;
    outputCode += "if ("+whileIfPredicat+")\n";
    outputCode += "{\n";
    outputCode += conditionsCode;
    outputCode += "if (" +ifPredicat+ ")\n";
    outputCode += "{\n";
    outputCode += actionsCode;
    outputCode += "\n{ //Subevents: \n";
    outputCode += codeGenerator.GenerateEventsListCode(game, scene, events, context);
    outputCode += "} //Subevents end.\n";
    outputCode += "}\n";
    outputCode += "} else stopDoWhile = true; \n";

    outputCode += "} while ( !stopDoWhile );\n";

    return outputCode;
}

vector < vector<gd::Instruction>* > WhileEvent::GetAllConditionsVectors()
{
    vector < vector<gd::Instruction>* > allConditions;
    allConditions.push_back(&whileConditions);
    allConditions.push_back(&conditions);

    return allConditions;
}

vector < vector<gd::Instruction>* > WhileEvent::GetAllActionsVectors()
{
    vector < vector<gd::Instruction>* > allActions;
    allActions.push_back(&actions);

    return allActions;
}

void WhileEvent::SaveToXml(TiXmlElement * eventElem) const
{
    //Save "While conditions"
    TiXmlElement * whileConditionsElem = new TiXmlElement( "WhileConditions" );
    eventElem->LinkEndChild( whileConditionsElem );
    OpenSaveGame::SaveConditions(whileConditions, whileConditionsElem);

    //Les conditions
    TiXmlElement * conditionsElem = new TiXmlElement( "Conditions" );
    eventElem->LinkEndChild( conditionsElem );
    OpenSaveGame::SaveConditions(conditions, conditionsElem);

    //Les actions
    TiXmlElement * actionsElem = new TiXmlElement( "Actions" );
    eventElem->LinkEndChild( actionsElem );
    OpenSaveGame::SaveActions(actions, actionsElem);

    //Sous �v�nements
    if ( !GetSubEvents().empty() )
    {
        TiXmlElement * subeventsElem;
        subeventsElem = new TiXmlElement( "Events" );
        eventElem->LinkEndChild( subeventsElem );

        OpenSaveGame::SaveEvents(events, subeventsElem);
    }
}


void WhileEvent::LoadFromXml(const TiXmlElement * eventElem)
{
    if ( eventElem->FirstChildElement( "WhileConditions" ) != NULL )
        OpenSaveGame::OpenConditions(whileConditions, eventElem->FirstChildElement( "WhileConditions" ));
    else
        cout << "Aucune informations sur les while conditions d'un �v�nement While";

    //Conditions
    if ( eventElem->FirstChildElement( "Conditions" ) != NULL )
        OpenSaveGame::OpenConditions(conditions, eventElem->FirstChildElement( "Conditions" ));
    else
        cout << "Aucune informations sur les conditions d'un �v�nement";

    //Actions
    if ( eventElem->FirstChildElement( "Actions" ) != NULL )
        OpenSaveGame::OpenActions(actions, eventElem->FirstChildElement( "Actions" ));
    else
        cout << "Aucune informations sur les actions d'un �v�nement";

    //Subevents
    if ( eventElem->FirstChildElement( "Events" ) != NULL )
        OpenSaveGame::OpenEvents(events, eventElem->FirstChildElement( "Events" ));
}

/**
 * Render the event in the bitmap
 */
void WhileEvent::Render(wxDC & dc, int x, int y, unsigned int width, EventsEditorItemsAreas & areas, EventsEditorSelection & selection)
{
    EventsRenderingHelper * renderingHelper = EventsRenderingHelper::GetInstance();
    int border = renderingHelper->instructionsListBorder;
    const int repeatHeight = 20;

    //Draw header rectangle
    InstructionsMetadataHolder metadataHolder; //TODO : For now, construct a wrapper around ExtensionsManager
    int whileConditionsHeight = renderingHelper->GetRenderedConditionsListHeight(whileConditions, width-80-border*2, metadataHolder)+border*2;
    wxRect headerRect(x, y, width, whileConditionsHeight+repeatHeight);
    renderingHelper->DrawNiceRectangle(dc, headerRect);

    //While text
    dc.SetFont( renderingHelper->GetNiceFont().Bold()  );
    dc.SetTextForeground(wxColour(0,0,0));
    dc.DrawText( _("Tant que :"), x+5, y+5 );

    //Draw "while conditions"
    renderingHelper->DrawConditionsList(whileConditions, dc, x+80+border, y+border, width-80-border*2, this, areas, selection, metadataHolder);

    dc.SetFont( renderingHelper->GetNiceFont().Bold()  );
    dc.SetTextForeground(wxColour(0,0,0));
    dc.DrawText( _("R�p�ter :"), x+4, y+whileConditionsHeight+3);
    whileConditionsHeight += repeatHeight;

    //Draw conditions rectangle
    wxRect rect(x, y+whileConditionsHeight, renderingHelper->GetConditionsColumnWidth()+border, GetRenderedHeight(width)-whileConditionsHeight);
    renderingHelper->DrawNiceRectangle(dc, rect);

    renderingHelper->DrawConditionsList(conditions, dc,
                                        x+border,
                                        y+whileConditionsHeight+border,
                                        renderingHelper->GetConditionsColumnWidth()-border, this, areas, selection, metadataHolder);
    renderingHelper->DrawActionsList(actions, dc,
                                     x+renderingHelper->GetConditionsColumnWidth()+border,
                                     y+whileConditionsHeight+border,
                                     width-renderingHelper->GetConditionsColumnWidth()-border*2, this, areas, selection, metadataHolder);
}

unsigned int WhileEvent::GetRenderedHeight(unsigned int width) const
{
    if ( eventHeightNeedUpdate )
    {
        EventsRenderingHelper * renderingHelper = EventsRenderingHelper::GetInstance();
        int border = renderingHelper->instructionsListBorder;
        const int repeatHeight = 20;

        //Get maximum height needed
        InstructionsMetadataHolder metadataHolder; //TODO : For now, construct a wrapper around ExtensionsManager
        int whileConditionsHeight = renderingHelper->GetRenderedConditionsListHeight(whileConditions, width-80-border*2, metadataHolder);
        int conditionsHeight = renderingHelper->GetRenderedConditionsListHeight(conditions, renderingHelper->GetConditionsColumnWidth()-border, metadataHolder);
        int actionsHeight = renderingHelper->GetRenderedActionsListHeight(actions, width-renderingHelper->GetConditionsColumnWidth()-border*2, metadataHolder);

        renderedHeight = (( conditionsHeight > actionsHeight ? conditionsHeight : actionsHeight ) + whileConditionsHeight + repeatHeight)+border*2+border*2;
        eventHeightNeedUpdate = false;
    }

    return renderedHeight;
}

gd::BaseEvent::EditEventReturnType WhileEvent::EditEvent(wxWindow* parent_, Game & game_, Scene & scene_, MainEditorCommand & mainEditorCommand_)
{
    return ChangesMade;
}

/**
 * Initialize from another WhileEvent.
 * Used by copy ctor and assignement operator
 */
void WhileEvent::Init(const WhileEvent & event)
{
    events = CloneVectorOfEvents(event.events);

    whileConditions = event.whileConditions;
    conditions = event.conditions;
    actions = event.actions;
}

/**
 * Custom copy operator
 */
WhileEvent::WhileEvent(const WhileEvent & event) :
BaseEvent(event)
{
    Init(event);
}

/**
 * Custom assignement operator
 */
WhileEvent& WhileEvent::operator=(const WhileEvent & event)
{
    if ( this != &event )
    {
        BaseEvent::operator=(event);
        Init(event);
    }

    return *this;
}

#endif
