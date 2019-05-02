#pragma once
#include <vector>
#define DEAD false
#define ALIVE true

// A cell class (that's our bacterium)
typedef class CCell
{
private:
	struct POS			// The position in a coordinate system
	{
		int x;
		int y;
	} m_Pos;

public:
	CCell() : m_Pos({ 0, 0 }) { }
	CCell(int x, int y) : m_Pos({ x, y }) { }

	int GetX() const { return m_Pos.x; }
	int GetY() const { return m_Pos.y; }
} *LPCELL;

// A cell colony class (gathering large amount of bacteria)
class CCellColony
{
private:
	std::vector<CCell> m_vCells;
	// This is a rectangle describing a precise position of our colony. The program will bind its calculation only
	// to this rect to get better performance.
	struct OWN_RECT
	{
		int left, top, right, bottom;
		OWN_RECT(int a = 0, int b = 0, int c = 0, int d = 0)
		{
			left = a;
			top = b; 
			right = c;
			bottom = d;
		}
	} m_rcSize;
	bool m_bLiveMode = false;

public:
	CCellColony() : m_rcSize(OWN_RECT()) { }
	CCellColony(std::vector<CCell> vData)
	{
		// Find a minimal rect in which a colony is fully included.
		m_rcSize = OWN_RECT();
		for (auto item : vData)
		{
			if (item.GetX() < m_rcSize.left) m_rcSize.left = item.GetX();
			if (item.GetY() < m_rcSize.top) m_rcSize.top = item.GetY();
			if (item.GetX() > m_rcSize.right) m_rcSize.right = item.GetX();
			if (item.GetY() > m_rcSize.bottom) m_rcSize.bottom = item.GetY();
		}
	}

	void UpdateRegion()
	{
		// Update the region of a colony.
		for (auto item : m_vCells)
		{
			if (item.GetX() < m_rcSize.left) m_rcSize.left = item.GetX();
			if (item.GetY() < m_rcSize.top) m_rcSize.top = item.GetY();
			if (item.GetX() > m_rcSize.right) m_rcSize.right = item.GetX();
			if (item.GetY() > m_rcSize.bottom) m_rcSize.bottom = item.GetY();
		}/*
		m_rcSize.left *= -1; 
		m_rcSize.top *= -1;*/
	}

	// Update and return a number of updated cells.
	int UpdateCells()
	{
		if (!m_bLiveMode) return 0;
		// A number of updated cells.
		int nResult = 0;

		// This is only one possible region to update.
		OWN_RECT UpdateRect = { m_rcSize.left - 1, m_rcSize.top - 1, m_rcSize.right + 1, m_rcSize.bottom + 1 };
		int nWidth = abs(UpdateRect.right) + abs(UpdateRect.left) + 1;
		int nHeight = abs(UpdateRect.bottom) + abs(UpdateRect.top) + 1;

		int** ppUpdateTable = new int* [nWidth];
		for (int i = 0; i < nWidth; ++i) ppUpdateTable[i] = new int[nHeight];

		for (int i = 0; i < nWidth; ++i)
			for (int j = 0; j < nHeight; ++j)
				ppUpdateTable[i][j] = 0;

		// Fill the update table with legitimate values representing a number of neighbours of every field in the update region.
		for (auto item : m_vCells)
		{
			// Consider a square 3x3, a cell called 'item' is the middle of the square.
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					if (i == 0 && j == 0) continue;
					ppUpdateTable[item.GetX() + i - m_rcSize.left + 1][item.GetY() + j - m_rcSize.top + 1]++;
				}
			}
		}

		// Create new cells on fields with exactly three neighbours. Dont have to consider all fields in the update region,
		// but only those from the previous step.
		int nSize = m_vCells.size();
		for (int t = 0; t < nSize; ++t)
		{
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					int x = m_vCells[t].GetX();
					int y = m_vCells[t].GetY();
					if (ppUpdateTable[x + i - m_rcSize.left + 1][y + j - m_rcSize.top + 1] == 3)
					{
						if (this->Spare(x + i, y + j))
						{
							m_vCells.push_back(CCell(x + i, y + j));
							nResult++;
						}
					}
				}
			}
		}
		
		// Kill cells that lived and had too many or too few neighbours.
		int nEnd = m_vCells.size();
		for (int i = 0; i < nEnd; ++i)
		{
			if (ppUpdateTable[m_vCells[i].GetX() - m_rcSize.left + 1][m_vCells[i].GetY() - m_rcSize.top + 1] == 2 ||
				ppUpdateTable[m_vCells[i].GetX() - m_rcSize.left + 1][m_vCells[i].GetY() - m_rcSize.top + 1] == 3) continue;
			std::swap(m_vCells[i], m_vCells[nEnd - 1]);
			nResult--;
			nEnd--;
		}
		m_vCells.resize(nEnd);

		// Update the region of a colony.
		this->UpdateRegion();
		
		// Delete the update table.
		for (int i = 0; i < nWidth; ++i) delete[] ppUpdateTable[i];
		delete[] ppUpdateTable;

		// Return a result
		return nResult;
	}

	// Check a content of a field.
	bool Spare(int x, int y)
	{
		for (auto item : m_vCells) if (item.GetX() == x && item.GetY() == y) return false;
		return true;
	}

	// Add cell.
	void AddCell(int x, int y)
	{
		// Check if a cell is in a colony.
		for (auto item : m_vCells) if (item.GetX() == x && item.GetY() == y) return;
		m_vCells.push_back(CCell(x, y));
		this->UpdateRegion();
	}

	// And remove the last cell.
	void RemoveCell()
	{
		if (!this->Alive()) return;
		m_vCells.pop_back();
		this->UpdateRegion();
	}

	// Check if our colony has at least one cell alive.
	bool Alive() const
	{
		if (m_vCells.size() == 0) return false;
		return true;
	}

	// The size of our colony.
	int Size() const
	{
		return m_vCells.size();
	}

	// Get a position of a certain cell.
	int GetX(int i) const
	{
		return m_vCells[i].GetX();
	}

	int GetY(int i) const
	{
		return m_vCells[i].GetY();
	}

	// Start a simulation.
	void Start()
	{
		m_bLiveMode = true;
	}

	// End a simulation.
	void End()
	{
		m_bLiveMode = false;
		m_vCells.clear();
		this->UpdateRegion();
	}
};

// The global cell colony
extern CCellColony g_CellColony;