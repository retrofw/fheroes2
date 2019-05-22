/*************************************************************************** 
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         * 
 *   This program is free software; you can redistribute it and/or modify  * 
 *   it under the terms of the GNU General Public License as published by  * 
 *   the Free Software Foundation; either version 2 of the License, or     * 
 *   (at your option) any later version.                                   * 
 *                                                                         * 
 *   This program is distributed in the hope that it will be useful,       * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        * 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         * 
 *   GNU General Public License for more details.                          * 
 *                                                                         * 
 *   You should have received a copy of the GNU General Public License     * 
 *   along with this program; if not, write to the                         * 
 *   Free Software Foundation, Inc.,                                       * 
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             * 
 ***************************************************************************/

#include <cstring>
#include "castle.h"
#include "race.h"
#include "monster.h"
#include "buildinginfo.h"
#include "settings.h"
#include "payment.h"

#ifdef WITH_XML
#include "xmlccwrap.h"
#endif

struct paymentstats_t
{
    const char* id;
    cost_t cost;
};

static paymentstats_t _payments[] = {
    { "buy_boat",       { 1000,10, 0, 0, 0, 0, 0 } },
    { "buy_spell_book", {  500, 0, 0, 0, 0, 0, 0 } },
    { "recruit_hero",   { 2500, 0, 0, 0, 0, 0, 0 } },

    { NULL, { 0, 0, 0, 0, 0, 0, 0 } },
};

void PaymentLoadCost(payment_t & payment, const cost_t & cost)
{
    payment.gold = cost.gold;
    payment.wood = cost.wood;
    payment.mercury = cost.mercury;
    payment.ore = cost.ore;
    payment.sulfur = cost.sulfur;
    payment.crystal = cost.crystal;
    payment.gems = cost.gems;
}

#ifdef WITH_XML
void LoadCostFromXMLElement(cost_t & cost, const TiXmlElement & element)
{
    int value;

    element.Attribute("gold", &value); cost.gold = value;
    element.Attribute("wood", &value); cost.wood = value;
    element.Attribute("mercury", &value); cost.mercury = value;
    element.Attribute("ore", &value); cost.ore = value;
    element.Attribute("sulfur", &value); cost.sulfur = value;
    element.Attribute("crystal", &value); cost.crystal = value;
    element.Attribute("gems", &value); cost.gems = value;
}
#endif

void PaymentConditions::UpdateCosts(const std::string & spec)
{
#ifdef WITH_XML
    // parse payments.xml
    TiXmlDocument doc;
    const TiXmlElement* xml_payments = NULL;

    if(doc.LoadFile(spec.c_str()) &&
        NULL != (xml_payments = doc.FirstChildElement("payments")))
    {
	paymentstats_t* ptr = &_payments[0];

        while(ptr->id)
        {
            const TiXmlElement* xml_payment = xml_payments->FirstChildElement(ptr->id);

            if(xml_payment)
        	LoadCostFromXMLElement(ptr->cost, *xml_payment);

            ++ptr;
        }
    }
    else
    VERBOSE(spec << ": " << doc.ErrorDesc());
#endif
}

PaymentConditions::BuyMonster::BuyMonster(u8 monster)
{
    Monster::GetCost(monster, *this);
}

PaymentConditions::UpgradeMonster::UpgradeMonster(u8 monster)
{
    Monster::GetUpgradeCost(monster, *this);
}

PaymentConditions::BuyBuilding::BuyBuilding(u8 race, u32 build)
{
    BuildingInfo::GetCost(build, race, *this);
}

PaymentConditions::BuyBoat::BuyBoat()
{
    paymentstats_t* ptr = &_payments[0];

    while(ptr->id && std::strcmp("buy_boat", ptr->id)) ++ptr;

    if(ptr) PaymentLoadCost(*this, ptr->cost);
}

PaymentConditions::BuySpellBook::BuySpellBook()
{
    paymentstats_t* ptr = &_payments[0];

    while(ptr->id && std::strcmp("buy_spell_book", ptr->id)) ++ptr;

    if(ptr) PaymentLoadCost(*this, ptr->cost);
}

PaymentConditions::RecruitHero::RecruitHero()
{
    paymentstats_t* ptr = &_payments[0];

    while(ptr->id && std::strcmp("recruit_hero", ptr->id)) ++ptr;

    if(ptr) PaymentLoadCost(*this, ptr->cost);
}
