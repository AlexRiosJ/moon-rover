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

        float wheelAngle;
         
        // Wheels offsets.
        float wheelsBackOffsetZ;
        float wheelsMidOffsetZ;
        float wheelsFrontOffsetZ;

        float wheelRightBackOffsetX;
        float wheelRightMidOffsetX;
        float wheelRightFrontOffsetX;

        float wheelLeftBackOffsetX;
        float wheelLeftMidOffsetX;
        float wheelLeftFrontOffsetX;

        // Rover position
        float x;
        float y;
        float z;

        // Rover rotation
        float deg;

        // For animation
        float wheelRotation;
        float currentDirection;
        float currentDirectionAnimation;
    public:
        Rover();
        ~Rover();
        void load();
        void bind(GLuint, GLuint, GLuint, GLuint);
        void draw(GLuint);

        void setPosition(float x, float y, float z);

        void getWheelLeftBackXZPosition(float *);
        void getWheelLeftMidXZPosition(float *);
        void getWheelLeftFrontXZPosition(float *);
        
        void getWheelRightBackXZPosition(float *);
        void getWheelRightMidXZPosition(float *);
        void getWheelRightFrontXZPosition(float *);

        void rotateWheels(int);
        void setDirection(float);
        void setRotation(float);

        float getDirection();
};
