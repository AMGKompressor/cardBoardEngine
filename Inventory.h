#pragma once

#include "LootConfig.h"

struct InventorySlot
{
	bool occupied = false;
	LootTier tier = LootTier::Grey;
	int value = 0;
};

class Inventory
{
public:
	static constexpr int kSlotCount = 3;

	void selectSlot(int index);
	int selectedSlot() const { return m_selected; }

	bool addLoot(LootTier tier, int value);
	bool removeSelected(LootTier& outTier, int& outValue);
	bool slotOccupied(int index) const;
	const InventorySlot& slot(int index) const;

private:
	InventorySlot m_slots[kSlotCount] = {};
	int m_selected = 0;
};
