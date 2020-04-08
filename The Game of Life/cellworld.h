#pragma once
#include <sstream>
#include <Windows.h>
#include <vector>
#include <fstream>
#define DEAD false
#define ALIVE true
#define MAX_DIST 20

// A cell class (that's our bacterium)
typedef class CCell
{
private:
	struct POS			// The position in a coordinate system
	{
		int x;
		int y;
	} m_Pos;
	int m_nNeighs = 0;

public:
	CCell() : m_Pos({ 0, 0 }) { }
	CCell(int x, int y) : m_Pos({ x, y }) { }

	int GetX() const { return m_Pos.x; }
	int GetY() const { return m_Pos.y; }
	int& Neighbours() { return m_nNeighs; }

	void operator=(const CCell& a)
	{
		m_Pos.x = a.GetX();
		m_Pos.y = a.GetY();
		m_nNeighs = a.m_nNeighs;
	}
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
	int m_nLastUpdate = 0;
	int m_nMaxSpeed = 1;
	int m_nGeneration = 0;

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
		if (m_vCells.size() == 0)
		{
			m_rcSize = { 0, 0, 0, 0 };
			return;
		}
		int nLeft = m_vCells[0].GetX(), nTop = m_vCells[0].GetY();
		int nRight = nLeft, nBottom = nTop;
		// Update the region of a colony.
		for (auto item : m_vCells)
		{
			if (item.GetX() < nLeft) nLeft = item.GetX();
			if (item.GetY() < nTop) nTop = item.GetY();
			if (item.GetX() > nRight) nRight = item.GetX();
			if (item.GetY() > nBottom) nBottom = item.GetY();
		}
		m_rcSize = { nLeft, nTop, nRight, nBottom };
	}

	// Update and return a number of updated cells.
	void UpdateCells(bool bForce = false)
	{
		if (m_vCells.size() == 0) m_bLiveMode = false;
		if (!bForce && !m_bLiveMode) return;
		// A number of updated cells.
		int nResult = 0;

		// This is only one possible region to update.
		OWN_RECT UpdateRect = { m_rcSize.left - 1, m_rcSize.top - 1, m_rcSize.right + 1, m_rcSize.bottom + 1 };
		int nWidth = abs(UpdateRect.right - UpdateRect.left) + 1;
		int nHeight = abs(UpdateRect.bottom - UpdateRect.top) + 1;

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

		int nSwap = m_vCells.size() - 1;
		for (int i = 0; i <= nSwap; ++i)
		{
			// Take a cell and read a number of its neighbours from the update table.
			auto& item = m_vCells[i];
			auto& swapper = m_vCells[nSwap];
			int nValue = ppUpdateTable[item.GetX() - m_rcSize.left + 1][item.GetY() - m_rcSize.top + 1];
			int nSwapperValue = ppUpdateTable[swapper.GetX() - m_rcSize.left + 1][swapper.GetY() - m_rcSize.top + 1];
			if (nValue != 2 && nValue != 3)
			{
				if (nSwapperValue == 2 || nSwapperValue == 3)
				{
					std::swap(item, swapper);
					nSwap--;
					nResult--;
				}
				else
				{
					nSwap--;
					i--;
				}
			}
		}
		m_vCells.resize(nSwap + 1);

		// Update the region of a colony.
		this->UpdateRegion();
		
		// Delete the update table.
		for (int i = 0; i < nWidth; ++i) delete[] ppUpdateTable[i];
		delete[] ppUpdateTable;

		// Return a result
		m_nLastUpdate = nResult;
		if (nResult > m_nMaxSpeed) m_nMaxSpeed = nResult;

		// Increment generation
		m_nGeneration++;
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
		if (!this->Alive() || m_bLiveMode) return;
		m_vCells.pop_back();
		this->UpdateRegion();
	}

	// Check if our colony has at least one cell alive.
	bool Alive() const
	{
		if (m_vCells.size() == 0) return false;
		return true;
	}

	// Load cells from file
	void Load(std::string strFileName)
	{
		this->End();

		std::vector<std::string> vLines;
		std::ifstream hFile(strFileName);
		std::string strLine;

		unsigned uLine = 0;

		while (std::getline(hFile, strLine)) {
			for (int i = 0; i < strLine.length(); ++i) {
				if (strLine[i] == 'O') AddCell(i, uLine);
			}
			uLine++;
		}
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
		m_nGeneration = 0;
	}

	// Start & pause a simulation.
	void Pause()
	{
		if (m_vCells.size() == 0) return;
		if (m_bLiveMode) m_bLiveMode = false;
		else m_bLiveMode = true;
	}

	// Return amount of updated cells;
	int GrowthSpeed() const
	{
		return m_nLastUpdate;
	}

	// Return max speed
	int MaxSpeed() const
	{
		return m_nMaxSpeed;
	}

	// Get current generation
	int Generation() const 
	{
		return m_nGeneration;
	}
};

// The global cell colony
extern CCellColony g_CellColony;