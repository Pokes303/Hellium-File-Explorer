#include "../main.hpp"
#include "button.hpp"

class Menu{
    private:
        std::vector<Button*> bv;
    public:
        Menu();
        ~Menu();

        Button* AddButton(Button* b);
        void RenderAll();
};