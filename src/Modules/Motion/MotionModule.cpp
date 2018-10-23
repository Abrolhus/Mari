#include "MotionModule.h"
#include "Core/Utils/Math.h"
#include "Core/Utils/SimpleTimer.h"
#include <iostream>
#include "Core/InitManager.h"

using namespace std;

MotionModule::MotionModule(SpellBook *spellBook) : Module(spellBook, "Motion", 0)
{
    SetHighPriority(true);
    InitManager::GetBlackboard()->thread.configCallbacks["motion"];
    motion = new rUNSWiftMotionAdapter();
    vx = vy = vth = 0;
}

MotionModule::~MotionModule()
{
    delete motion;
}

void MotionModule::OnStart()
{
    motion->Start();
}

void MotionModule::OnStop()
{
    motion->Stop();
}

void MotionModule::Load()
{
    LOAD(motion);
    LOAD(behaviour);
}

void MotionModule::Save()
{
    
}

void MotionModule::Tick(float ellapsedTime)
{
    ActionCommand::All request;
    if(stiff != spellBook->motion.Stiff)
    {
        stiff = spellBook->motion.Stiff;
        request.stiffen = stiff ? ActionCommand::STIFFEN : ActionCommand::NONE;
    }
    else if(stand != spellBook->motion.Stand)
    {
        stand = spellBook->motion.Stand;
        request.body = ActionCommand::Body(stand ? ActionCommand::Body::STAND : ActionCommand::Body::INITIAL);
    }
    else if(stand != spellBook->motion.GoalieStand)
    {
        stand = spellBook->motion.GoalieStand;
        request.body = ActionCommand::Body(stand ? ActionCommand::Body::GOALIE_STAND : ActionCommand::Body::GOALIE_INITIAL);
    }
    else if(spellBook->motion.Dead)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::DEAD);
    }
    else if(spellBook->motion.TipOver)
    {
        //request.body = ActionCommand::Body(ActionCommand::Body::TIP_OVER);
        request.body = ActionCommand::Body(ActionCommand::Body::DEAD);
    }
    else if(spellBook->motion.GetupFront)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::GETUP_FRONT);
    }
    else if(spellBook->motion.GetupBack)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::GETUP_BACK);
    }
    else if(spellBook->motion.Crouch)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::WALK, 0, 0, 0, 0.4f, 1.0f);
    }
    else if(spellBook->motion.GoalieSit)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::GOALIE_SIT);
    }
    else if(spellBook->motion.GoalieCentre)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::GOALIE_CENTRE);
    }
    else if(spellBook->motion.GoalieUncentre)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::GOALIE_UNCENTRE);
    }
    else if(spellBook->motion.DefenderCentre)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::DEFENDER_CENTRE);
    }
    else if(spellBook->motion.KickLeft)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::KICK);
        request.body.foot = ActionCommand::Body::LEFT;
        request.body.caughtRight = true;
        request.body.caughtLeft = true;
    }
    else if(spellBook->motion.KickRight)
    {
        request.body = ActionCommand::Body(ActionCommand::Body::KICK);
        request.body.foot = ActionCommand::Body::RIGHT;
        request.body.caughtRight = true;
        request.body.caughtLeft = true;
    }
    else if(spellBook->motion.Walk)
    {
        request.head = ActionCommand::Head(spellBook->motion.HeadYaw, spellBook->motion.HeadPitch, !spellBook->motion.HeadRelative, spellBook->motion.HeadSpeed, 0.2f);
        vx = spellBook->motion.Vx * 1000.0f;
        vy = spellBook->motion.Vy * 1000.0f;
        vth = spellBook->motion.Vth * 1000.0f;
        if(vx != 0 || vy != 0 || vth != 0)
        {
            ScaleWalk2014(&vx, &vy, &vth);
            request.body = ActionCommand::Body(ActionCommand::Body::WALK, vx, vy, vth);
        }
        else
            request.body = ActionCommand::Body(ActionCommand::Body::WALK, 0, 0, 0, 0.4f, 1.0f);
        request.body.caughtRight = spellBook->motion.LimpRight;
        request.body.caughtLeft = spellBook->motion.LimpLeft;
    }
    request.leds.leftEye.red = ((spellBook->behaviour.LeftEye & 0xFF0000) >> 16) > 128;
    request.leds.leftEye.green = ((spellBook->behaviour.LeftEye & 0x00FF00) >> 8) > 128;
    request.leds.leftEye.blue = ((spellBook->behaviour.LeftEye & 0x0000FF)) > 128;
    request.leds.rightEye.red = ((spellBook->behaviour.RightEye & 0xFF0000) >> 16) > 128;
    request.leds.rightEye.green = ((spellBook->behaviour.RightEye & 0x00FF00) >> 8) > 128;
    request.leds.rightEye.blue = ((spellBook->behaviour.RightEye & 0x0000FF)) > 128;
    motion->Tick(request);
}

void MotionModule::ScaleWalk2014(float *forward, float *left, float *turn)
{
    float f = *forward;
    float l = *left;
    float t = *turn;
    float sum = abs(f/300.0f) + abs(l/200.0f) + abs(t/1.5f);
    if(sum <= 1)
        return;
    *forward = round(f/sum);
    *left = round(l/sum);
    *turn = round(t/sum);
}