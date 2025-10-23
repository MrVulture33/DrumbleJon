#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"


Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(96, 96);
	return true;
}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	//L10: TODO 3; Load the spritesheet of the player
	texture = Engine::GetInstance().textures->Load("Assets/Textures/player2_spritesheet.png");

	//L10: TODO 3: Load the spritesheet animations from the TSX file
	std::unordered_map<int, std::string> animNames = { {0,"idle"},{11,"move"},{22,"jump"} };
	anims.LoadFromTSX("Assets/Textures/player2_spritesheet.tsx", animNames);
	anims.SetCurrent("idle");

	// L08 TODO 5: Add physics to the player - initialize physics body
	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");

	return true;
}

bool Player::Update(float dt)
{
	Physics* physics = Engine::GetInstance().physics.get();

	// Read current velocity
	b2Vec2 velocity = physics->GetLinearVelocity(pbody);
	velocity = { 0, velocity.y }; // Reset horizontal velocity

	// Move left/right
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		anims.SetCurrent("move");
		velocity.x = -speed;
		playerfacingLeft = true;
		//L10: TODO 6: Update the animation based on the player's state

	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		anims.SetCurrent("move");
		velocity.x = speed;
		playerfacingLeft = false;
		//L10: TODO 6: Update the animation based on the player's state

	}

	// Jump (impulse once)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
		physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		//L10: TODO 6: Update the animation based on the player's state
		anims.SetCurrent("jump");
		isJumping = true;
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_M) == KEY_DOWN && isDashing == false && isJumping == true) {
		if (playerfacingLeft)
		{
			physics->ApplyLinearImpulseToCenter(pbody, -dashForce, -0.5f, true);
		}
		else
		{
			physics->ApplyLinearImpulseToCenter(pbody, dashForce, -0.5f, true);
		}
		velocity.x = dashfactor * speed;
		//L10: TODO 6: Update the animation based on the player's state
		isDashing = true;
	}

	// Preserve vertical speed while jumping
	if (isJumping == true) {
		velocity.y = physics->GetYVelocity(pbody);
	}

	if (isDashing == true) {
		velocity.x = physics->GetXVelocity(pbody);
	}

	// Apply velocity via helper
	physics->SetLinearVelocity(pbody, velocity);

	// L10: TODO 5: Update the animation based on the player's state (moving, jumping, idle)
	anims.Update(dt);
	SDL_Rect animFrame = anims.GetCurrentFrame();

	// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	//L10: TODO 7: Center the camera on the player
	float limitleft = Engine::GetInstance().render->camera.w / 2;
	float limitright = Engine::GetInstance().map->GetMapSizeInPixels().getX() - Engine::GetInstance().render->camera.w / 2;

	Engine::GetInstance().render->camera.y = -position.getY() + Engine::GetInstance().render->camera.h / 2;
	if (position.getX() - limitleft > 0 && position.getX() < limitright) {
		Engine::GetInstance().render->camera.x = -position.getX() + Engine::GetInstance().render->camera.w / 2;
	}

	// L10: TODO 5: Draw the player using the texture and the current animation frame
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);
	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		isDashing = false;
		//L10: TODO 6: Update the animation based on the player's state
		anims.SetCurrent("idle");
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

