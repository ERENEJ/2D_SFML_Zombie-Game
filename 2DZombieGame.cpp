#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "2DZombieGame.h"
#include "Player.h"
#include <sstream>
#include <fstream>
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"

using namespace sf;

//TODO move or change this logic object pool alike logic or just use Vector
//want to create Bullet array at the heap
//moved out of main it was exceeding 16kb stack size for more information see:
//https://learn.microsoft.com/en-us/cpp/code-quality/c6262?view=msvc-170  and
//https://stackoverflow.com/questions/64456303/error-code-c6262-consider-moving-data-to-heap
Bullet bullets[100];


int main()
{
    TextureHolder texHolder;

    enum class State
    {
        PAUSED, LEVELING_UP,GAME_OVER, PLAYING
    };

    State state = State::GAME_OVER;

    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;

    RenderWindow window(VideoMode(resolution.x, resolution.y), "ZombieGame", Style::Fullscreen);

    View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

    Clock clock;

    Time gameTimeTotal;

    Vector2f mouseWorldPosition;

    Vector2i mouseScreenPosition;

    Player player;

    IntRect arena;

    VertexArray background;
    Texture textureBackGround = TextureHolder::GetTexture("graphics/background_sheet.png");
    

    int numZombies;
    int numZombiesAlive;
    Zombie* zombieArray = nullptr;

    //Bullet bullets[100];
    int currentBullet = 0;
    int bulletsSpare = 24;
    int bulletsInClip = 6;
    int clipSize = 6;
    float fireRate = 1;

    //time of last firing event
    Time lastPressed;


    window.setMouseCursorVisible(false);
    Sprite spriteCrosshair;
    Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");

    spriteCrosshair.setTexture(textureCrosshair);
    spriteCrosshair.setOrigin(25, 25);
    
    Pickup healthPickup(1);
    Pickup ammoPickup(2);

    int score = 0;
    int hiScore = 0;

    
    Sprite spriteGameOver;
    Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
    spriteGameOver.setTexture(textureGameOver);
    spriteGameOver.setPosition(0, 0);

    View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));

    Sprite spriteAmmoIcon;
    Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
    spriteAmmoIcon.setTexture(textureAmmoIcon);
    spriteAmmoIcon.setPosition(120, 980);

    Font font;
    font.loadFromFile("fonts/zombiecontrol.ttf");

    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(155);
    pausedText.setFillColor(Color::White);
    pausedText.setPosition(400, 400);
    pausedText.setString("Press Enter \nto continue");

    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(125);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(250, 850);
    gameOverText.setString("Press Enter to play");

    Text levelUpText;
    levelUpText.setFont(font);
    levelUpText.setCharacterSize(80);
    levelUpText.setFillColor(Color::White);
    levelUpText.setPosition(150, 250);
    std::stringstream levelUpStream;
    levelUpStream <<
        "1- Increased rate of fire" <<
        "\n2- Increased clip size(next reload)" <<
        "\n3- Increased max health" <<
        "\n4- Increased run speed" <<
        "\n5- More and better health pickups" <<
        "\n6- More and better ammo pickups";
    levelUpText.setString(levelUpStream.str());

    Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(55);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(200, 980);

    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(55);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(20, 0);

    std::ifstream inputFile("gamedata/scores.txt");
    if (inputFile.is_open())
    {
        inputFile >> hiScore;
        inputFile.close();
    }

    Text hiScoreText;
    hiScoreText.setFont(font);
    hiScoreText.setCharacterSize(55);
    hiScoreText.setFillColor(Color::White);
    hiScoreText.setPosition(1400, 0);
    std::stringstream s;
    s << "Hi Score:" << hiScore;
    hiScoreText.setString(s.str());

    
    Text zombiesRemainingText;
    zombiesRemainingText.setFont(font);
    zombiesRemainingText.setCharacterSize(55);
    zombiesRemainingText.setFillColor(Color::White);
    zombiesRemainingText.setPosition(1500, 980);
    zombiesRemainingText.setString("Zombies: 100");

    
    int wave = 0;
    Text waveNumberText;
    waveNumberText.setFont(font);
    waveNumberText.setCharacterSize(55);
    waveNumberText.setFillColor(Color::White);
    waveNumberText.setPosition(1250, 980);
    waveNumberText.setString("Wave: 0");

    // Health bar
    RectangleShape healthBar;
    healthBar.setFillColor(Color::Red);
    healthBar.setPosition(450, 980);

    int framesSinceLastHUDUpdate = 0;

    // How often (in frames) should we update the HUD
    int fpsMeasurementFrameInterval = 1000;

    // Prepare the hit sound
    SoundBuffer hitBuffer;
    hitBuffer.loadFromFile("sound/hit.wav");
    Sound hit;
    hit.setBuffer(hitBuffer);


    SoundBuffer splatBuffer;
    splatBuffer.loadFromFile("sound/splat.wav");
    sf::Sound splat;
    splat.setBuffer(splatBuffer);

  
    SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("sound/shoot.wav");
    Sound shoot;
    shoot.setBuffer(shootBuffer);

  
    SoundBuffer reloadBuffer;
    reloadBuffer.loadFromFile("sound/reload.wav");
    Sound reload;
    reload.setBuffer(reloadBuffer);

    SoundBuffer reloadFailedBuffer;
    reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
    Sound reloadFailed;
    reloadFailed.setBuffer(reloadFailedBuffer);

 
    SoundBuffer powerupBuffer;
    powerupBuffer.loadFromFile("sound/powerup.wav");
    Sound powerup;
    powerup.setBuffer(powerupBuffer);

    // Prepare the pickup sound
    SoundBuffer pickupBuffer;
    pickupBuffer.loadFromFile("sound/pickup.wav");
    Sound pickup;
    pickup.setBuffer(pickupBuffer);

    while(window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Return && state == State::PLAYING)
                {
                    state = State::PAUSED;
                }
                
                else if (event.key.code == Keyboard::Return && state == State::PAUSED)
                {
                    state = State::PLAYING;

                    // restarting the clock here to prevent frame skip
                    clock.restart();
                }

                else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
                {
                    state = State::LEVELING_UP;
                    wave = 0;
                    score = 0;

                    currentBullet = 0;
                    bulletsSpare = 24;
                    bulletsInClip = 6;
                    clipSize = 6;
                    fireRate = 1;

                    player.resetPlayerStats();

                }
                if (state == State::PLAYING)
                {   
                    if (event.key.code == Keyboard::R)
                    {
                        //for reload this logic can be adjusted
                        if (bulletsSpare >= clipSize)
                        {
                            bulletsInClip = clipSize;
                            bulletsSpare -= clipSize;
                            reload.play();
                        }

                        else if (bulletsSpare > 0)
                        {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0;
                            reload.play();
                        }

                        else
                        {
                            reloadFailed.play();
                        }
                    }
                   
                }

            }//end of key pressed detection

        }//end of event poll

        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }

        //movement logic
        if (state == State::PLAYING)
        {
            if (Keyboard::isKeyPressed(Keyboard::W))
            {
                player.moveUp();
            }
            else
            {
                player.stopUp();
            }
            if (Keyboard::isKeyPressed(Keyboard::S))
            {
                player.moveDown();
            }
            else
            {
                player.stopDown();
            }

            if (Keyboard::isKeyPressed(Keyboard::A))
            {
                player.moveLeft();
            }
            else
            {
                player.stopLeft();
            }

            if (Keyboard::isKeyPressed(Keyboard::D))
            {
                player.moveRight();
            }
            else
            {
                player.stopRight();
            }

            if (Mouse::isButtonPressed(Mouse::Left))
            {
                if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0)
                {
                    bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
                    currentBullet++;
                    if (currentBullet > 99)
                    {
                        currentBullet = 0;
                    }

                    shoot.play();

                    lastPressed = gameTimeTotal;
                    bulletsInClip--;
                }
            }//end of fire weapon event
        }

        if (state == State::LEVELING_UP)
        {
           if (event.key.code == Keyboard::Num1)
           {    
               fireRate++;
               state = State::PLAYING;
           }

           if (event.key.code == Keyboard::Num2)
           {    
               clipSize += clipSize;
               state = State::PLAYING;
           }

           if (event.key.code == Keyboard::Num3)
           {    
               player.upgradeHealth();
               state = State::PLAYING;
           }

           if (event.key.code == Keyboard::Num4)
           {    
               player.upgradeSpeed();
               state = State::PLAYING;
           }

           if (event.key.code == Keyboard::Num5)
           {    
               healthPickup.upgrade();
               state = State::PLAYING;
           }

           if (event.key.code == Keyboard::Num6)
           {    
               ammoPickup.upgrade();
               state = State::PLAYING;
           }

           // Level Logic setup logic
           if (state == State::PLAYING)
           {    
               wave++;

               arena.width = 500 * 3;
               arena.height = 500 * 3;
               arena.left = 0;
               arena.top = 0;

               int tileSize = createBackground(background, arena);

               player.spawn(arena, resolution, tileSize);

               healthPickup.setArena(arena);
               ammoPickup.setArena(arena);

               numZombies = 5 * wave;

               //deleting previously allocated zombie array
               delete[] zombieArray;
               zombieArray = createHorde(numZombies, arena);
               numZombiesAlive = numZombies;

               powerup.play();

               clock.restart();
           }
        }// end of leveling up

        if (state == State::PLAYING)
        {   
            ///////////// update ///////////////////
            
            //delta time
            Time dt = clock.restart();

            gameTimeTotal += dt;

            float dtAsSeconds = dt.asSeconds();

            mouseScreenPosition = Mouse::getPosition();

            mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);

            spriteCrosshair.setPosition(mouseWorldPosition);

            player.update(dtAsSeconds, Mouse::getPosition());
            Vector2f playerPosition(player.getCenter());

            mainView.setCenter(player.getCenter());

            //loop through each zombie and update them
            for (int i = 0; i < numZombies; i++)
            {
                if (zombieArray[i].isAlive())
                {
                    zombieArray[i].update(dt.asSeconds(), playerPosition);
                }
            }

            //100 bullet array size
            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    bullets[i].update(dtAsSeconds);
                }
            }

            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);

            // Collision detection via intersects method
            //magic number 100 is the bullet array size
            for (int i = 0; i < 100; i++)
            {
                for (int j = 0; j < numZombies; j++)
                {
                    
                    if (bullets[i].isInFlight() && zombieArray[j].isAlive())
                    {
                      

                        if (bullets[i].getPosition().intersects(zombieArray[j].getPosition()))
                        {
                           
                            // Stop the bullet
                            bullets[i].stop();

                            // call hit (OnHit event)
                            if (zombieArray[j].hit()) 
                            {   
                                
                                score += 10;

                                if (score >= hiScore)
                                {
                                    hiScore = score;
                                }

                                numZombiesAlive--;
                                // end raund
                                if (numZombiesAlive == 0) 
                                {
                                    state = State::LEVELING_UP;
                                }
                            }

                            splat.play();
                        }
                    }

                }
            }// End of bullet collision detection

            for (int i = 0; i < numZombies; i++)
            {
                if (player.getPosition().intersects(zombieArray[i].getPosition()) && zombieArray[i].isAlive())
                {
                    if (player.hit(gameTimeTotal))
                    {
                        hit.play();
                    }

                    if (player.getHealth() <= 0)
                    {   
                        
                        state = State::GAME_OVER;
                        std::ofstream outputFile("gamedata/scores.txt");
                        outputFile << hiScore;
                        outputFile.close();
                    }
                }
            }//end of player zombie collision detection

             // Has the player touched health pickup
            if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
            {
                player.increaseHealthLevel(healthPickup.onCollision());
                pickup.play();

            }

            // Has the player touched ammo pickup
            if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
            {
                bulletsSpare += ammoPickup.onCollision();
                reload.play();
            }

            // size up the health bar
            healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

            // Increment the number of frames since the last HUD calculation
            framesSinceLastHUDUpdate++;
            // Calculate FPS every fpsMeasurementFrameInterval frames
            if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
            {

                // Update game HUD text
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssHiScore;
                std::stringstream ssWave;
                std::stringstream ssZombiesAlive;

                // Update the related info

                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());

               
                ssScore << "Score:" << score;
                scoreText.setString(ssScore.str());

               
                ssHiScore << "Hi Score:" << hiScore;
                hiScoreText.setString(ssHiScore.str());

              
                ssWave << "Wave:" << wave;
                waveNumberText.setString(ssWave.str());

         
                ssZombiesAlive << "Zombies:" << numZombiesAlive;
                zombiesRemainingText.setString(ssZombiesAlive.str());

                framesSinceLastHUDUpdate = 0;
            }

            //////////////////// draw //////////////

            window.clear();
            window.setView(mainView);

            window.draw(background, &textureBackGround);

            for (int i = 0; i < numZombies; i++)
            {
                window.draw(zombieArray[i].getSprite());
            }

            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    window.draw(bullets[i].getShape());
                }
            }
            
            
            window.draw(player.getSprite());

            if (ammoPickup.isSpawned())
            {
                window.draw(ammoPickup.getSprite());
            }
            if (healthPickup.isSpawned())
            {
                window.draw(healthPickup.getSprite());
            }

            window.draw(spriteCrosshair);
       
            window.setView(hudView);

            window.draw(spriteAmmoIcon);
            window.draw(ammoText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            window.draw(healthBar);
            window.draw(waveNumberText);
            window.draw(zombiesRemainingText);
            
        }
        
        if (state == State::LEVELING_UP)
        {
            window.draw(spriteGameOver);
            window.draw(levelUpText);
        }

        if (state == State::PAUSED)
        {
            window.draw(pausedText);
        }

        if (state == State::GAME_OVER)
        {
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(hiScoreText);
        }
 

        window.display();
    }// window while

    //only resource that allocated at runtime must be freed
    // opsys will also delete al reserved area but still good practice
    delete[] zombieArray;
    return 0;
}


