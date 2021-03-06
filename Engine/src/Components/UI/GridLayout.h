#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include "BoxLayout.h"

namespace Ossium
{
    
    struct GridLayoutSchema : public Schema<GridLayoutSchema, 20>
    {
        DECLARE_BASE_SCHEMA(GridLayoutSchema, 20);
        
        // Total rows in the grid.
        M(unsigned int, rows) = 0;

        // Total columns in the grid.
        M(unsigned int, cols) = 0;

    };

    /// Creates a grid of BoxLayouts for a specified number of rows and columns.
    class GridLayout : public LayoutComponent, public GridLayoutSchema
    {
    public:
        CONSTRUCT_SCHEMA(LayoutComponent, GridLayoutSchema);
        DECLARE_COMPONENT(LayoutComponent, GridLayout);

        void OnDestroy();

        void OnLoadFinish();
        
        // Update the cells in the grid.
        void LayoutRefresh();

        // Returns a pointer to the BoxLayout instance placed in a particular cell.
        BoxLayout* GetCellElement(unsigned int col, unsigned int row);

        // Returns a pointer to the BoxLayout instance placed in a particular cell. Returns nullptr if world position is outside.
        // Also optionally takes a column index and row index pointer to indicate the column and row used.
        BoxLayout* GetCellElementAt(Vector2 worldPos, unsigned int* outcol = nullptr, unsigned int* outrow = nullptr);

    private:
        // BoxLayout objects in the grid.
        std::vector<std::vector<BoxLayout*>> cellElements;

    };
    
}

#endif // GRIDLAYOUT_H
