#include "Inventory.h"

void Inventory::selectSlot(int index)
{
	if (index < 0 || index >= kSlotCount)
	{
		return;
	}
	m_selected = index;
}

bool Inventory::slotOccupied(int index) const
{
	if (index < 0 || index >= kSlotCount)
	{
		return false;
	}
	return m_slots[index].occupied;
}

const InventorySlot& Inventory::slot(int index) const
{
	static InventorySlot empty;
	if (index < 0 || index >= kSlotCount)
	{
		return empty;
	}
	return m_slots[index];
}

bool Inventory::addLoot(LootTier tier, int value)
{
	if (m_slots[m_selected].occupied == false)
	{
		m_slots[m_selected].occupied = true;
		m_slots[m_selected].tier = tier;
		m_slots[m_selected].value = value;
		return true;
	}

	for (int i = 0; i < kSlotCount; ++i)
	{
		if (!m_slots[i].occupied)
		{
			m_slots[i].occupied = true;
			m_slots[i].tier = tier;
			m_slots[i].value = value;
			m_selected = i;
			return true;
		}
	}

	return false;
}

bool Inventory::removeSelected(LootTier& outTier, int& outValue)
{
	if (!m_slots[m_selected].occupied)
	{
		return false;
	}

	outTier = m_slots[m_selected].tier;
	outValue = m_slots[m_selected].value;
	m_slots[m_selected].occupied = false;
	m_slots[m_selected].value = 0;
	return true;
}
