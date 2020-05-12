#include "Shape.hpp"

class Rover
{
    private:
        Shape chasis;
        Shape rockerLeft;
        Shape bogieLeft;
        Shape driveLeftBack;
        Shape driveLeftFront;
        Shape wheelLeftBack;
        Shape wheelLeftMid;
        Shape wheelLeftFront;
        Shape rockerRight;
        Shape bogieRight;
        Shape driveRightBack;
        Shape driveRightFront;
        Shape wheelRightBack;
        Shape wheelRightMid;
        Shape wheelRightFront;

        float angleX;
        float angleY;
        float angleZ;
    public:
        Rover();
        ~Rover();
        void Load();
        void Bind(GLuint, GLuint, GLuint, GLuint);
        void Draw(GLuint);
};
