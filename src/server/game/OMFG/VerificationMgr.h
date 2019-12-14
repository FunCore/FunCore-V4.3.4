/*
 * Copyright (C) 2013-2014 trinity core og
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OMFG_VERIFICATIONMGR_H
#define OMFG_VERIFICATIONMGR_H

class Player;

namespace VerificationMgr
{
    bool IsValidPlayerSpell(Player* player, uint32 spell);
    bool CheckAllPlayerSpells();
    bool IsSpellFitBySkillLine(uint32 spellId, uint32 raceMask, uint32 classMask, bool isPlayerAccount = true);
};

#endif // OMFG_VERIFICATIONMGR_H
