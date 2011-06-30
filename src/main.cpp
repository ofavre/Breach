/**
 * @file main.cpp
 *
 * @brief Main program.
 *
 * @section LICENSE
 *
 * Copyright (c) 2011 Olivier Favre
 *
 * This file is part of Breach.
 *
 * Licensed under the Simplified BSD License,
 * for details please see LICENSE file or the website
 * http://www.opensource.org/licenses/BSD-2-Clause
 */

//! Asks for definition of function prototypes for extensions
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/freeglut_ext.h>

#include <iostream>
#include <png.h>
#include <cmath>
#include <sys/time.h>
#include <vector>

using namespace std;

#include "PngImage.hpp"
#include "matrix.hpp"
#include "renderable.hpp"
#include "targets.hpp"
#include "walls.hpp"
#include "selection.hpp"

/*! \def MIN(a,b)
 * @brief A macro that returns the minimum of \a a and \a b.
 */
#define MIN(a,b) ((a)<=(b)?(a):(b))
/*! \def MAX(a,b)
 * @brief A macro that returns the maximum of \a a and \a b.
 */
#define MAX(a,b) ((a)>=(b)?(a):(b))


/*! \def TARGET_FPS
 * @brief A macro that defines the desired target FPS.
 */
#define TARGET_FPS 60
//! @brief Computed FPS to be displayed in overlay
int last_fps = 0;

// Textures ids
//! @brief Texture id for targets
GLuint target_texture = -1;
//! @brief Texture id for walls
GLuint wall_texture = -1;
//! @brief Texture id for the breach hole
GLuint breach_texture = -1;
//! @brief Texture id for outline of hidden breaches
GLuint breachhidden_texture = -1;

// Windowing stuff
//! @brief Scale used for passing to pixels to OpenGL unit
float pixelToUnitScale;
//! @brief Window's content width
int windowWidth;
//! @brief Window's content height
int windowHeight;
//! @brief Desired minimum viewport width in OpenGL unit
float width;
//! @brief Desired minimum viewport height in OpenGL unit
float height;

//! @brief Tracks (last) mouse buttons state
bool mouseButtonPressed[3] = {false, false, false};
//! @brief Last mouse button manipulated
int lastMouseButton;
//! @brief Last mouse known position
int lastMouseX;
//! @brief Last mouse known position
int lastMouseY;

//! Player speed
float playerSpeed = .01f;
/** @brief Player inclinaison speed.
 *
 * Used to control how much to rotate at each mousewheel step.
 */
float playerInclinaisonSpeed = .1f;
//! @brief Player looking direction
Matrix<float,4,1> playerLookAt ((float[4]){0, 0, -1, 1});
//! @brief Player position
Matrix<float,4,1> playerPosition ((float[4]){0, 0, .75f, 1});
//! @brief Player inclinaison vector (towards the current up)
Matrix<float,4,1> playerInclinaison ((float[4]){0, 1, 0, 1});
/** @brief Player moving directions.
 *
 * One value per axis.
 * Should be between -1 and 1.
 * We use this array to track the desired moving directions,
 * as we disabled key repeats (for smooth movement).
 */
int playerAdvance[3] = {0, 0, 0};

//! @brief Default breach width
const float BREACH_WIDTH = 0.4;
//! @brief Default breach height
const float BREACH_HEIGHT = 0.4;
/** @brief Defines a breach.
 */
struct Breach {
    //! @brief A breach can be either opened or closed (not yet shot)
    bool opened;
    /**
     * @brief The transformation matrix to apply to translate, orient and scale the breach.
     *
     * After applying this transformation,
     * the breach must be rendered as a (possibly tesseled) quad,
     * using -1/+1 for X and Y, and 0 for Z.
     */
    Matrix<float,4,4> transformation;
};
//! @brief The defined breaches
Breach breaches[2] = {
    { true,  Matrix<float,4,4>((float[]){BREACH_WIDTH,0,0,0, 0,BREACH_HEIGHT,0,0, 0,0,1,0, -.5,.5,-2,1}) },
    { false, Matrix<float,4,4>() }
};



/**
 * @brief Renders the scene primitives.
 *
 * @param forSelection Whether the scene should be rendered for selection test
 *                     using names, or for normal rendering using colors.
 */
void draw_scene(bool forSelection = false) {
    if (!forSelection) {

        if (breaches[0].opened) {

            // Test fake far scene (simply draw a target behind the wall)
            GLfloat mat_ambiant[] = { 1, 1, 1, 1 };
            GLfloat mat_diffuse[] = { 1, 1, 1, 1 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambiant);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glColor4f(1.0, 1.0, 1.0, 1.0);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.75f);
            glBindTexture(GL_TEXTURE_2D, target_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            float x = 0, y = 0, z = -3, size = 1.9/2;
            glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x-size, y-size, z);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(x+size, y-size, z);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(x+size, y+size, z);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(x-size, y=size, z);
            glEnd();
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);

        }
    }
    // Make the framebuffer all opaque
    glColor4f(0,0,0,1);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (!forSelection) {
        if (breaches[0].opened) {
            // Draw breach in alpha only
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, breach_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glMultMatrixf(breaches[0].transformation.values);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0);
            glVertex3f(-1, -1, 0);
            glTexCoord2f(1,0);
            glVertex3f( 1, -1, 0);
            glTexCoord2f(1,1);
            glVertex3f( 1,  1, 0);
            glTexCoord2f(0,1);
            glVertex3f(-1,  1, 0);
            glEnd();
            glPopMatrix();
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_LIGHTING);
            // Draw wall, blending according to previous (destination) alpha
            glClear(GL_DEPTH_BUFFER_BIT);
        }
    }
    // (Draw the wall even if there is no breach on it, or if we are in selection mode)
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    wallsRenderer->fullRender(forSelection ? GL_SELECT : GL_RENDER);
    glDisable(GL_BLEND);
    if (!forSelection) {
        // Make the framebuffer all opaque again // not sure it's useful
        glColor4f(0,0,0,1);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    if (!forSelection) {
        // Draw lines from the wall to the targets
        glColor4f(1.0, 1.0, 1.0, 1.0);
        for (vector<Target>::iterator it = targets.begin() ; it < targets.end() ; it++) {
            Target& t = *it;
            float x = t.getX();
            float y = t.getY();
            float z = t.getZ();

            glNormal3f(0, 0, 1);
            glBegin(GL_LINES);
            glVertex3f(x, y, -2);
            glVertex3f(x, y, z);
            glEnd();
        }
    }

    targetsRenderer->fullRender(forSelection ? GL_SELECT : GL_RENDER);

    if (!forSelection) {
        if (breaches[0].opened) {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
            // Draw the outline of the hidden breach
            GLfloat mat_ambiant[] = { 10, 5, 0, 1 };
            GLfloat mat_diffuse[] = { 10, 5, 0, 1 };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambiant);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            glNormal3f(0, 0, 1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, breachhidden_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glMultMatrixf(breaches[0].transformation.values);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(0, -10);

            // Appear on top of occulting objects
            glDisable(GL_CULL_FACE);
            glDepthFunc(GL_GREATER);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0);
            glVertex3f(-1, -1, 0);
            glTexCoord2f(1,0);
            glVertex3f( 1, -1, 0);
            glTexCoord2f(1,1);
            glVertex3f( 1,  1, 0);
            glTexCoord2f(0,1);
            glVertex3f(-1,  1, 0);
            glEnd();

            // Appear directly onto the porting wall (only when seen from the cull face)
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glDepthFunc(GL_LESS);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0);
            glVertex3f(-1, -1, 0);
            glTexCoord2f(1,0);
            glVertex3f( 1, -1, 0);
            glTexCoord2f(1,1);
            glVertex3f( 1,  1, 0);
            glTexCoord2f(0,1);
            glVertex3f(-1,  1, 0);
            glEnd();
            glCullFace(GL_BACK);
            glDepthFunc(GL_LESS);

            glPolygonOffset(0, 0);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glPopMatrix();
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }

}

/**
 * @brief Draws the scene.
 *
 * @param forSelection Whether the scene should be rendered for selection test
 *                     using names, or for normal rendering using colors.
 */
void doDisplay(bool forSelection = false) {
    // Move player
    if (playerAdvance[0] != 0 || playerAdvance[1] != 0 || playerAdvance[2] != 0) {
        playerPosition = playerPosition + (playerLookAt*playerAdvance[0] - playerInclinaison*playerLookAt*playerAdvance[1] + playerInclinaison*playerAdvance[2]) * playerSpeed;
    }
    // Compute the absolute look-at point
    float playerLookAtReal[3];
    for (int i = 0 ; i < 3 ; i++) playerLookAtReal[i] = playerPosition(i,0) + playerLookAt(i,0);

    // Configure the view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(playerPosition(0,0), playerPosition(1,0), playerPosition(2,0), playerLookAtReal[0], playerLookAtReal[1], playerLookAtReal[2], playerInclinaison(0,0), playerInclinaison(1,0), playerInclinaison(2,0));

    if (!forSelection) {
        // Buffers reinitialisation
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // General configuration
        glShadeModel(GL_SMOOTH);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glEnable(GL_DEPTH_TEST);

        // Configure a positionnal light
        GLfloat light_ambient[] = { 0, 0, 0, 1 };
        GLfloat light_diffuse[] = { 1, 1, 1, 1 };
        GLfloat light_specular[] = { 1, 1, 1, 1 };
        //GLfloat light_position[] = { playerPosition[0], playerPosition[1], playerPosition[2], 1 }; // the player sheds its own light
        GLfloat light_position[] = { 0, 0, 0, 1 };
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, .5);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1);
        // Configure a global light
        GLfloat lmodel_ambient[] = { .1, .1, .1, 1 };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
        //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }

    glEnable(GL_CULL_FACE);

    draw_scene(forSelection);

    if (!forSelection) {
        // 2D Overlay
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_INVERT);
        glRasterPos2d(windowWidth-60, windowHeight-20);
        char fps_str[10];
        sprintf(fps_str, "%d FPS", last_fps);
        for (char* i = fps_str; *i != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *i);
        }
        glDisable(GL_COLOR_LOGIC_OP);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        //glFlush(); // for GLUT_SINGLE buffer
        glutSwapBuffers(); // for GLUT_DOUBLE buffer
    }
}

/**
 * @brief Handles display, drawing the scene and slowing down frame rate.
 */
void display() {
    static timeval lastcall = {0,0};

    doDisplay(false);

    // Attempt to respect a maximum frame rate
    timeval thiscall;
    gettimeofday(&thiscall, NULL);
    long elapsed = (thiscall.tv_sec-lastcall.tv_sec)*1000000+thiscall.tv_usec-lastcall.tv_usec;
    static long total_elapsed = 0;
    static int frame_count = 0;
    frame_count++;
    total_elapsed += elapsed;
    if (total_elapsed > 1e6/2) {
        last_fps = frame_count / (total_elapsed/1e6f);
        frame_count = 0;
        total_elapsed = 0;
    }
    lastcall = thiscall;
    long stilltowait = 1.0e6/TARGET_FPS - elapsed;
    if (stilltowait > 0) usleep(stilltowait);
    glutPostRedisplay();
}

/**
 * @brief Figure out what object has been clicked and take the appropriate action.
 *
 * Called upon a mouse click at a specified position.
 * Currently only affects targets.
 *
 * @param x Absciss of the clicked pixel
 * @param y Ordinate of the clicked pixel
 */
void doSelection(int x, int y) {
    GLuint buffer[512];
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glSelectBuffer(sizeof(buffer)/sizeof(*buffer), buffer);
    glRenderMode(GL_SELECT);
    glInitNames();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Zoom on the very pixel the mouse is onto
    gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y), 1.0f, 1.0f, viewport);
    gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]), 0.01f, 10.0f);

    // Render the scene for selection
    doDisplay(true);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    SelectionUtil selection = SelectionUtil::finishGlSelection();
    vector<SelectionUtil::Hit> hits = selection.getHits();

    printf("%lu hits ! (including walls...)\n", hits.size());
    if (!hits.empty()) {
        for (vector<SelectionUtil::Hit>::iterator it = hits.begin() ; it < hits.end() ; ++it) {
            SelectionUtil::Hit& hit = *it;
            printf (" number of names for hit = %lu\n", hit.nameHierarchy.size());
            printf("  z1 is %g", hit.zMin);
            printf(" z2 is %g\n", hit.zMax);
            printf ("  the name is:");
            for (vector<GLuint>::iterator itn = hit.nameHierarchy.begin() ; itn < hit.nameHierarchy.end() ; ++itn) {
                GLuint name = *itn;
                printf (" %u", name);
                if (itn == hit.nameHierarchy.begin() && name == 1) {
                    printf("(targets)");
                } else if (itn == hit.nameHierarchy.begin() && name == 2) {
                    printf("(walls)");
                }
            }
            printf ("\n");
        }

        TypedSelectionVisitor<Target> targetSelectionResolver(hits[0].nameHierarchy);
        targetsRenderer->accept(targetSelectionResolver);

        if (targetSelectionResolver.isSelectedObjectFound()) {
            Target* shotTarget = targetSelectionResolver.getSelectedObject();
            printf("Found : %p at (%f, %f, %f)\n", shotTarget, shotTarget->getX(), shotTarget->getY(), shotTarget->getZ());
            shotTarget->setHit();
        } else
            printf("Not target hit\n");
    }
}

/**
 * @brief Handle mouse button press/release.
 *
 * Currently starts/stops camera orientation and selection.
 *
 * @param button Mouse button manipulated
 * @param state Whether the button has been pressed or released
 * @param x Absciss of the mouse pointer when the event was issued
 * @param y Ordinate of the mouse pointer when the event was issued
 */
void mouse(int button, int state, int x, int y) {
    lastMouseButton = button;
    if (button == GLUT_RIGHT_BUTTON) {
        // Prepare for camera movement
        mouseButtonPressed[2] = state == GLUT_DOWN;
        lastMouseX = x;
        lastMouseY = y;
    } else if (button == GLUT_LEFT_BUTTON) {
        if (mouseButtonPressed[0] && state == GLUT_UP) {
            mouseButtonPressed[0] = false;
        } else if (!mouseButtonPressed[0] && state == GLUT_DOWN) {
            // First time event of button being pressed
            mouseButtonPressed[0] = true;
            // Launch a selection test
            doSelection(x, y);
        }
    } else if (state == GLUT_DOWN && (button == 3 || button == 4)) {
        // Rotate inclinaison with mouse wheel
        Matrix<float,4,4> rot = MatrixHelper::rotation(playerInclinaisonSpeed*(button == 4 ? 1 : -1), playerLookAt);
        playerInclinaison = rot * playerInclinaison;
    }
}
/**
 * @brief Handle mouse motion (button state staying unmodified).
 *
 * Currently only affects camera orientation.
 *
 * @param x Absciss of the mouse pointer when the event was issued
 * @param y Ordinate of the mouse pointer when the event was issued
 */
void motion(int x, int y) {
    if (mouseButtonPressed[2]) {
        // Rotate the camera using last recorded position
        double angleX = (x-lastMouseX) / 300.0 / 2.0;
        double angleY = (y-lastMouseY) / 300.0 / 2.0;
        Matrix<float,4,4> rotX = MatrixHelper::rotation(angleX, playerInclinaison);
        Matrix<float,4,4> rotY = MatrixHelper::rotation(angleY, playerLookAt*playerInclinaison);
        playerLookAt = rotY * rotX * playerLookAt;
        playerInclinaison = rotY * playerInclinaison;
        lastMouseX = x;
        lastMouseY = y;
    }
}
/**
 * @brief Handle key press.
 *
 * Currently only affects player motion.
 *
 * @param key Character pressed.
 *            For special keys not corresponding to characters, use \code glutSpecialFunc \endcode instead.
 * @param x Absciss of the mouse pointer when the event was issued
 * @param y Ordinate of the mouse pointer when the event was issued
 */
void keyboard(unsigned char key, int x, int y) {
    x = y = 0; // suppress unused warning
    // Control player motion
    if (key == 'z') {
        playerAdvance[0] = MIN(1, playerAdvance[0]+1);
    } else if (key == 'q') {
        playerAdvance[1] = MAX(-1, playerAdvance[1]-1);
    } else if (key == 's') {
        playerAdvance[0] = MAX(-1, playerAdvance[0]-1);
    } else if (key == 'd') {
        playerAdvance[1] = MIN(1, playerAdvance[1]+1);
    } else if (key == 'a') {
        playerAdvance[2] = MIN(1, playerAdvance[2]+1);
    } else if (key == 'e') {
        playerAdvance[2] = MAX(-1, playerAdvance[2]-1);
    }
}
/**
 * @brief Handle key release.
 *
 * Currently only affects player motion.
 *
 * @param key Character released.
 *            For special keys not corresponding to characters, use \code glutSpecialUpFunc \endcode instead.
 * @param x Absciss of the mouse pointer when the event was issued
 * @param y Ordinate of the mouse pointer when the event was issued
 */
void keyboardUp(unsigned char key, int x, int y) {
    x = y = 0; // suppress unused warning
    // Control player motion
    if (key == 'z') {
        playerAdvance[0] = MAX(-1, playerAdvance[0]-1);
    } else if (key == 'q') {
        playerAdvance[1] = MIN(1, playerAdvance[1]+1);
    } else if (key == 's') {
        playerAdvance[0] = MIN(1, playerAdvance[0]+1);
    } else if (key == 'd') {
        playerAdvance[1] = MAX(-1, playerAdvance[1]-1);
    } else if (key == 'a') {
        playerAdvance[2] = MAX(-1, playerAdvance[2]-1);
    } else if (key == 'e') {
        playerAdvance[2] = MIN(1, playerAdvance[2]+1);
    }
}

/**
 * @brief Handle window resize.
 *
 * @param w The new window's content width
 * @param h The new window's content height
 */
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    printf("%d x %d\n", w, h);
    // Reconfigure the viewport and perspective
    glViewport(0, 0, (GLsizei) windowWidth, (GLsizei) windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    pixelToUnitScale = MIN(width/windowWidth, height/windowHeight);
    // Preserve aspect pixel ratio of 1:1
    gluPerspective(45.0, w/(GLdouble)h, 0.01, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * @brief Program entrypoint.
 *
 * Configures and runs OpenGL.
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    // Initialisation sugar, optional
    // Disactivated because:
    //  1) It activates no used feature
    //  2) It breaks compatibility with legacy and poor opensource GPU drivers
    //glutInitContextVersion(3, 2); // 3.2 is the maximum value still avoiding Compiz blinking
    //glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    //glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    //glutInitContextFlags(GLUT_DEBUG);
    //glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_COMPATIBILITY_PROFILE);
    //glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    //glutInitContextProfile(GLUT_CORE_PROFILE);

    // Configure OpenGL and register callbacks
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowSize(600, 600); // Size of the OpenGL window
    glutCreateWindow("Breach"); // Creates OpenGL Window
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutIgnoreKeyRepeat(1);

    // Load textures
    GLuint texs[4];
    glGenTextures(4, texs);
    // Target
    PngImage* pi_target = new PngImage();
    pi_target->read_from_file("resources/target.png");
    target_texture = texs[0];
    Texture targetTexture (target_texture, pi_target->getGLInternalFormat(), pi_target->getWidth(), pi_target->getHeight(), pi_target->getGLFormat(), pi_target->getTexels());
    // Wall
    PngImage* pi_wall = new PngImage();
    pi_wall->read_from_file("resources/brushed-walls.png");
    wall_texture = texs[1];
    Texture wallTexture (wall_texture, GL_RGB8, pi_wall->getWidth(), pi_wall->getHeight(), GL_RGB, pi_wall->getTexels());
    // Breach hole, alpha only
    PngImage* pi_breach = new PngImage();
    pi_breach->read_from_file("resources/breach-alpha.png");
    breach_texture = texs[2];
    glBindTexture(GL_TEXTURE_2D, breach_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, pi_breach->getWidth(), pi_breach->getHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, pi_breach->getTexels());
    // Hidden breach outline
    PngImage* pi_breachhidden = new PngImage();
    pi_breachhidden->read_from_file("resources/breach-hidden.png");
    breachhidden_texture = texs[3];
    glBindTexture(GL_TEXTURE_2D, breachhidden_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pi_breachhidden->getWidth(), pi_breachhidden->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pi_breachhidden->getTexels());
    // Free textures as they have been transferred to the GPU
    delete pi_target;
    pi_target = NULL;
    delete pi_wall;
    pi_wall = NULL;
    delete pi_breach;
    pi_breach = NULL;
    delete pi_breachhidden;
    pi_breachhidden = NULL;

    initTargets(targetTexture);
    initWalls(wallTexture);

    // Let OpenGL control the program through its main loop
    glutMainLoop();

    std::cout << "Bye!" << std::endl;
    return 0;
}
