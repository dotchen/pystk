
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2011-2015 Joerg Henrichs, Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifdef WIN32
#  ifdef __CYGWIN__
#    include <unistd.h>
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  ifdef _MSC_VER
#    include <direct.h>
#  endif
#else
#  include <signal.h>
#  include <unistd.h>
#endif
#include <stdexcept>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <limits>

#include <IEventReceiver.h>

#include "pystk.hpp"
#include "main_loop.hpp"
#include "achievements/achievements_manager.hpp"
#include "addons/addons_manager.hpp"
#include "addons/news_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "font/font_manager.hpp"
#include "graphics/camera.hpp"
#include "graphics/camera_debug.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/frame_buffer.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/referee.hpp"
#include "graphics/render_target.hpp"
#include "graphics/rtts.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/dialog_queue.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "input/wiimote_manager.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/network_item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/combined_characteristic.hpp"
#include "karts/controller/ai_base_lap_controller.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "modes/profile_world.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "network/rewind_queue.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/profile_manager.hpp"
#include "online/request_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "replay/replay_recorder.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/register_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "states_screens/dialogs/init_android_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/constants.hpp"
#include "utils/crash_reporting.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/mini_glm.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "utils/objecttype.h"
#include "util.hpp"

const PySTKGraphicsConfig & PySTKGraphicsConfig::hd() {
	static PySTKGraphicsConfig config = {600,400,
		false, true, true, true, true, 
		2,
		true,
		true,
		true,
		true,
		true,
		true,
		1 | 2,
	};
	return config;
}
const PySTKGraphicsConfig & PySTKGraphicsConfig::sd() {
	static PySTKGraphicsConfig config = {600,400,
		false, false, false, false, false,
		2,
		true,
		true,
		true,
		true,
		true,
		true,
		1 | 2,
	};
	return config;
}
const PySTKGraphicsConfig & PySTKGraphicsConfig::ld() {
	static PySTKGraphicsConfig config = {600,400,
		false, false, false, false, false,
		0,
		false,
		false,
		false,
		false,
		false,
		false,
		0,
	};
	return config;
}

class PySTKRenderTarget {
	friend class PySuperTuxKart;

private:
	std::unique_ptr<RenderTarget> rt_;

protected:
	void render(irr::scene::ICameraSceneNode* camera, float dt);
	void fetch(std::shared_ptr<PySTKRenderData> data);
	
public:
	PySTKRenderTarget(std::unique_ptr<RenderTarget>&& rt);
	
};

PySTKRenderTarget::PySTKRenderTarget(std::unique_ptr<RenderTarget>&& rt):rt_(std::move(rt)) {
}
void PySTKRenderTarget::render(irr::scene::ICameraSceneNode* camera, float dt) {
	rt_->renderToTexture(camera, dt);
}
void PySTKRenderTarget::fetch(std::shared_ptr<PySTKRenderData> data) {
	RTT * rtts = rt_->getRTTs();
	if (rtts && data) {
		
		unsigned int W = rtts->getWidth(), H = rtts->getHeight();
		// Read the color and depth image
		data->width = W;
		data->height = H;
		data->color_buf_.resize(W*H*3);
		data->depth_buf_.resize(W*H);
		data->instance_buf_.resize(W*H);
		
		rtts->getFBO(FBO_COLOR_AND_LABEL).bind();
		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// Read color and depth
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, data->color_buf_.data());
		glReadPixels(0, 0, W, H, GL_DEPTH_COMPONENT, GL_FLOAT, data->depth_buf_.data());
		
		// Read the labels
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glReadPixels(0, 0, W, H, GL_RED_INTEGER, GL_INT, data->instance_buf_.data());
		
		
		// Flip all buffers (thank you OpenGL)
		yflip(data->color_buf_.data(), H, W*3);
		yflip(data->depth_buf_.data(), H, W);
		yflip(data->instance_buf_.data(), H, W);
	}
	
}


void PySTKAction::set(KartControl * control) const {
	control->setAccel(acceleration);
	control->setBrake(brake);
	control->setFire(fire);
	control->setNitro(nitro);
	control->setRescue(rescue);
	control->setSteer(steering_angle);
	control->setSkidControl(drift ? (steering_angle > 0 ? KartControl::SC_RIGHT : KartControl::SC_LEFT) : KartControl::SC_NONE);
}
void PySTKAction::get(const KartControl * control) {
	acceleration = control->getAccel();
	brake = control->getBrake();
	fire = control->getFire();
	nitro = control->getNitro();
	rescue = control->getRescue();
	steering_angle = control->getSteer();
	drift = control->getSkidControl() != KartControl::SC_NONE;
}

int PySuperTuxKart::n_running = 0;
bool PySuperTuxKart::render_window = 0;
void PySuperTuxKart::init(const PySTKGraphicsConfig & config) {
	if (n_running > 0)
		throw std::invalid_argument("Cannot init while supertuxkart is running!");
	initUserConfig();
	stk_config->load(file_manager->getAsset("stk_config.xml"));
	initGraphicsConfig(config);
	initRest();
}
void PySuperTuxKart::clean() {
	if (n_running > 0)
		throw std::invalid_argument("Cannot clean up while supertuxkart is running!");
	cleanSuperTuxKart();
	Log::flushBuffers();

#ifndef WIN32
	if (user_config) //close logfiles
	{
		Log::closeOutputFiles();
#endif
#ifndef ANDROID
		fclose(stderr);
		fclose(stdout);
#endif
#ifndef WIN32
	}
#endif
	delete file_manager;
	file_manager = NULL;
}
int PySuperTuxKart::nRunning() { return n_running; }
PySuperTuxKart::PySuperTuxKart(const PySTKRaceConfig & config) {
	if (n_running > 0)
		throw std::invalid_argument("Cannot run more than one supertux instance per process!");
	n_running++;
	
	resetObjectId();
	
	setupConfig(config);
	
	load();
	
	render_targets_.push_back( std::make_unique<PySTKRenderTarget>(irr_driver->createRenderTarget( {UserConfigParams::m_width, UserConfigParams::m_height}, "player0" )) );
}
std::vector<std::string> PySuperTuxKart::listTracks() {
	if (track_manager)
		return track_manager->getAllTrackIdentifiers();
	return std::vector<std::string>();
}
std::vector<std::string> PySuperTuxKart::listKarts() {
	if (kart_properties_manager)
		return kart_properties_manager->getAllAvailableKarts();
	return std::vector<std::string>();
}
PySuperTuxKart::~PySuperTuxKart() {
	if (input_manager) {
		delete input_manager;
		input_manager = NULL;
	}
	
	AchievementsManager::destroy();
	
	
    delete main_loop;
	main_loop = nullptr;
    Referee::cleanup();

    if(unlock_manager)          delete unlock_manager;
	unlock_manager = nullptr;
	
	
	n_running--;
	StateManager::get()->resetActivePlayers();
}

void PySuperTuxKart::start() {
	setupRaceStart();
	StateManager::get()->enterGameState();
	race_manager->setupPlayerKartInfo();
	race_manager->startNew(false);
	time_leftover_ = 0.f;
    if (config_.player_ai) {
		AbstractKart * player_kart = World::getWorld()->getPlayerKart(0);
		ai_controller_ = World::getWorld()->loadAIController(player_kart);
	}
}
void PySuperTuxKart::stop() {
	render_targets_.clear();
	if (CVS->isGLSL())
	{
		// Flush all command before delete world, avoid later access
		SP::SPTextureManager::get()
			->checkForGLCommand(true/*before_scene*/);
		// Reset screen in case the minimap was drawn
		glViewport(0, 0, irr_driver->getActualScreenSize().Width,
			irr_driver->getActualScreenSize().Height);
	}

	// In case the user opened a race pause dialog
	GUIEngine::ModalDialog::dismiss();

	if (World::getWorld())
	{
		race_manager->clearNetworkGrandPrixResult();
		race_manager->exitRace();
	}
	
	if (ai_controller_) delete ai_controller_;
	ai_controller_ = nullptr;
}
void PySuperTuxKart::render(float dt) {
//     m_wind->update();

    PropertyAnimator::get()->update(dt);
	SP::SPTextureManager::get()->checkForGLCommand();

	World *world = World::getWorld();

    if (world)
    {
		// Render all views
		for(unsigned int i = 0; i < Camera::getNumCameras() && i < render_targets_.size(); i++) {
			Camera::getCamera(i)->activate(false);
			render_targets_[i]->render(Camera::getCamera(i)->getCameraSceneNode(), dt);
		}
		while (render_data_.size() < render_targets_.size()) render_data_.push_back( std::make_shared<PySTKRenderData>() );
		// Fetch all views
		for(unsigned int i = 0; i < render_targets_.size(); i++) {
			render_targets_[i]->fetch(render_data_[i]);
		}
    }
}

bool PySuperTuxKart::step(const PySTKAction & a) {
	KartControl & control = World::getWorld()->getPlayerKart(0)->getControls();
	a.set(&control);
	return step();
}
bool PySuperTuxKart::step() {
	const float dt = config_.step_size;

	if (World::getWorld())
		World::getWorld()->updateGraphics(dt);
	
	// irr_driver->update alternative
	if (render_window) {
		irr_driver->update(dt);
	} else {
		irr_driver->minimalUpdate(dt);
	}
	render(dt);
	input_manager->update(dt);
	GUIEngine::update(dt);
	
	if (World::getWorld()) {
		time_leftover_ += dt;
		int ticks = stk_config->time2Ticks(time_leftover_);
		time_leftover_ -= stk_config->ticks2Time(ticks);
		for(int i=0; i<ticks; i++)
			World::getWorld()->updateWorld(1);
		// Update the AI control
		if (ai_controller_) {
			KartControl control;
			ai_controller_->setControls(&control);
			ai_controller_->update(ticks);
			ai_controller_->setControls(nullptr);
			ai_action_.get(&control);
		}
	}

	if (!irr_driver->getDevice()->run())
		return false;
	return race_manager && race_manager->getFinishedPlayers() < race_manager->getNumPlayers();
}

void PySuperTuxKart::load() {
	input_manager = new InputManager ();

	// Get into menu mode initially.
	input_manager->setMode(InputManager::MENU);
	main_loop = new MainLoop(0/*parent_pid*/);
	material_manager->loadMaterial();

	// Preload the explosion effects (explode.png)
	ParticleKindManager::get()->getParticles("explosion.xml");

	GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
														"options_video.png"));
	kart_properties_manager -> loadAllKarts    ();

	// Needs the kart and track directories to load potential challenges
	// in those dirs, so it can only be created after reading tracks
	// and karts.
	unlock_manager = new UnlockManager();
	AchievementsManager::create();

	// Reading the rest of the player data needs the unlock manager to
	// initialise the game slots of all players and the AchievementsManager
	// to initialise the AchievementsStatus, so it is done only now.
	PlayerManager::get()->initRemainingData();

	GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
														"gui_lock.png"  ) );
	projectile_manager->loadData();

	// Both item_manager and powerup_manager load models and therefore
	// textures from the model directory. To avoid reading the
	// materials.xml twice, we do this here once for both:
	file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""), "models");
	const std::string materials_file =
		file_manager->getAsset(FileManager::MODEL,"materials.xml");
	if(materials_file!="")
	{
		// Some of the materials might be needed later, so just add
		// them all permanently (i.e. as shared). Adding them temporary
		// will actually not be possible: powerup_manager adds some
		// permanent icon materials, which would (with the current
		// implementation) make the temporary materials permanent anyway.
		material_manager->addSharedMaterial(materials_file);
	}
	Referee::init();
	powerup_manager->loadPowerupsModels();
	ItemManager::loadDefaultItemMeshes();

	GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
														"gift.png")       );

	attachment_manager->loadModels();
	file_manager->popTextureSearchPath();

	GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
														"banana.png")    );
}

// ============================================================================
/** This function sets up all data structure for an immediate race start.
 *  It is used when the -N or -R command line options are used.
 */
void PySuperTuxKart::setupRaceStart()
{
    // Skip the start screen. This esp. means that no login screen is
    // displayed (if necessary), so we have to make sure there is
    // a current player
    PlayerManager::get()->enforceCurrentPlayer();

    InputDevice *device;

    // Use keyboard 0 by default in --no-start-screen
    device = input_manager->getDeviceManager()->getKeyboard(0);

    // Create player and associate player with keyboard
    StateManager::get()->createActivePlayer(
        PlayerManager::get()->getPlayer(0), device);

    if (!kart_properties_manager->getKart(UserConfigParams::m_default_kart))
    {
        Log::warn("main", "Kart '%s' is unknown so will use the "
            "default kart.",
            UserConfigParams::m_default_kart.c_str());
        race_manager->setPlayerKart(0,
                           UserConfigParams::m_default_kart.getDefaultValue());
    }
    else
    {
        // Set up race manager appropriately
        if (race_manager->getNumPlayers() > 0)
            race_manager->setPlayerKart(0, UserConfigParams::m_default_kart);
    }

    // ASSIGN should make sure that only input from assigned devices
    // is read.
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
}   // setupRaceStart

static RaceManager::MinorRaceModeType translate_mode(PySTKRaceConfig::RaceMode mode) {
	switch (mode) {
		case PySTKRaceConfig::NORMAL_RACE: return RaceManager::MINOR_MODE_NORMAL_RACE;
		case PySTKRaceConfig::TIME_TRIAL: return RaceManager::MINOR_MODE_TIME_TRIAL;
		case PySTKRaceConfig::FOLLOW_LEADER: return RaceManager::MINOR_MODE_FOLLOW_LEADER;
		case PySTKRaceConfig::THREE_STRIKES: return RaceManager::MINOR_MODE_3_STRIKES;
		case PySTKRaceConfig::FREE_FOR_ALL: return RaceManager::MINOR_MODE_FREE_FOR_ALL;
		case PySTKRaceConfig::CAPTURE_THE_FLAG: return RaceManager::MINOR_MODE_CAPTURE_THE_FLAG;
		case PySTKRaceConfig::SOCCER: return RaceManager::MINOR_MODE_SOCCER;
	}
	return RaceManager::MINOR_MODE_NORMAL_RACE;
}

void PySuperTuxKart::setupConfig(const PySTKRaceConfig & config) {
	config_ = config;
	
	race_manager->setDifficulty(RaceManager::Difficulty(config.difficulty));
	race_manager->setMinorMode(translate_mode(config.mode));
	
	if (config.kart.length()) {
		const KartProperties *prop = kart_properties_manager->getKart(config.kart);
		if (prop)
		{
			UserConfigParams::m_default_kart = config.kart;

			// if a player was added with -N, change its kart.
			// Otherwise, nothing to do, kart choice will be picked
			// up upon player creation.
			if (StateManager::get()->activePlayerCount() > 0)
			{
				race_manager->setPlayerKart(0, config.kart);
			}
			Log::verbose("main", "You chose to use kart '%s'.",
							config.kart.c_str());
		}
		else
		{
			Log::warn("main", "Kart '%s' not found, ignored.",
						config.kart.c_str());
		}
	}
	if (config.track.length())
		race_manager->setTrack(config.track);
	
	UserConfigParams::m_no_start_screen = true;
	UserConfigParams::m_race_now = true;
	
	race_manager->setNumLaps(config.laps);
	
	UserConfigParams::m_unlock_everything = 2;
	
// 	stk_config->setPhysicsFPS(30.);
// 	race_manager->setDefaultAIKartList(l);
// 	race_manager->setNumKarts( UserConfigParams::m_default_num_karts );
}

void PySuperTuxKart::initGraphicsConfig(const PySTKGraphicsConfig & config) {
	UserConfigParams::m_fullscreen = false;
	UserConfigParams::m_prev_width  = UserConfigParams::m_width  = config.screen_width;
	UserConfigParams::m_prev_height = UserConfigParams::m_height = config.screen_height;
	UserConfigParams::m_glow = config.glow;
	UserConfigParams::m_bloom = config.bloom;
	UserConfigParams::m_light_shaft = config.light_shaft;
	UserConfigParams::m_dynamic_lights = config.dynamic_lights;
	UserConfigParams::m_dof = config.dof;
	UserConfigParams::m_particles_effects = config.particles_effects;
	UserConfigParams::m_animated_characters = config.animated_characters;
	UserConfigParams::m_motionblur = config.motionblur;
	UserConfigParams::m_animated_characters = config.animated_characters;
	UserConfigParams::m_mlaa = config.mlaa;
	UserConfigParams::m_texture_compression=  config.texture_compression;
	UserConfigParams::m_ssao = config.ssao;
	UserConfigParams::m_degraded_IBL = config.degraded_IBL;
	UserConfigParams::m_high_definition_textures = config.high_definition_textures;
	render_window = config.render_window;
}


//=============================================================================
/** Initialises the minimum number of managers to get access to user_config.
 */
void PySuperTuxKart::initUserConfig()
{
    file_manager = new FileManager();
    user_config  = new UserConfig();     // needs file_manager
    user_config->loadConfig();
    // Some parts of the file manager needs user config (paths for models
    // depend on artist debug flag). So init the rest of the file manager
    // after reading the user config file.
    file_manager->init();
    if (UserConfigParams::m_language.toString() != "system")
    {
#ifdef WIN32
        std::string s=std::string("LANGUAGE=")
                     +UserConfigParams::m_language.c_str();
        _putenv(s.c_str());
#else
        setenv("LANGUAGE", UserConfigParams::m_language.c_str(), 1);
#endif
    }

    translations            = new Translations();   // needs file_manager
    stk_config              = new STKConfig();      // in case of --stk-config
                                                    // command line parameters
}   // initUserConfig

//=============================================================================
void PySuperTuxKart::initRest()
{
    SP::setMaxTextureSize();
    irr_driver = new IrrDriver();

    if (irr_driver->getDevice() == NULL)
    {
        Log::fatal("main", "Couldn't initialise irrlicht device. Quitting.\n");
    }

    StkTime::init();   // grabs the timer object from the irrlicht device

    // Now create the actual non-null device in the irrlicht driver
    irr_driver->initDevice();

    // Init GUI
    IrrlichtDevice* device = irr_driver->getDevice();
    video::IVideoDriver* driver = device->getVideoDriver();


    font_manager = new FontManager();
    font_manager->loadFonts();
    GUIEngine::init(device, driver, StateManager::get());

    // This only initialises the non-network part of the add-ons manager. The
    // online section of the add-ons manager will be initialised from a
    // separate thread running in network HTTP.
#ifndef SERVER_ONLY
    addons_manager = NULL;
    if (!ProfileWorld::isNoGraphics())
        addons_manager = new AddonsManager();
#endif
    Online::ProfileManager::create();

    // The request manager will start the login process in case of a saved
    // session, so we need to read the main data from the players.xml file.
    // The rest will be read later (since the rest needs the unlock- and
    // achievement managers to be created, which can only be created later).
    PlayerManager::create();
    Online::RequestManager::get()->startNetworkThread();
#ifndef SERVER_ONLY
    if (!ProfileWorld::isNoGraphics())
        NewsManager::get();   // this will create the news manager
#endif

    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties, which are defined in stk_config.
    history                 = new History              ();
    ReplayPlay::create();
    ReplayRecorder::create();
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    kart_properties_manager = new KartPropertiesManager();
    projectile_manager      = new ProjectileManager    ();
    powerup_manager         = new PowerupManager       ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();

    // The maximum texture size can not be set earlier, since
    // e.g. the background image needs to be loaded in high res.
    irr_driver->setMaxTextureSize();
    KartPropertiesManager::addKartSearchDir(
                 file_manager->getAddonsFile("karts/"));
    track_manager->addTrackSearchDir(
                 file_manager->getAddonsFile("tracks/"));

    {
        XMLNode characteristicsNode(file_manager->getAsset("kart_characteristics.xml"));
        kart_properties_manager->loadCharacteristics(&characteristicsNode);
    }

    track_manager->loadTrackList();

    GUIEngine::addLoadingIcon(irr_driver->getTexture(FileManager::GUI_ICON,
                                                     "notes.png"      ) );

    grand_prix_manager      = new GrandPrixManager     ();
    // Consistency check for challenges, and enable all challenges
    // that have all prerequisites fulfilled
    grand_prix_manager->checkConsistency();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                      "cup_gold.png"    ) );

    race_manager            = new RaceManager          ();
    // default settings for Quickstart
    race_manager->setNumPlayers(1);
    race_manager->setNumLaps   (3);
    race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    race_manager->setDifficulty(
                 (RaceManager::Difficulty)(int)UserConfigParams::m_difficulty);

    if (!track_manager->getTrack(UserConfigParams::m_last_track))
        UserConfigParams::m_last_track.revertToDefaults();

    race_manager->setTrack(UserConfigParams::m_last_track);
	kart_properties_manager -> loadAllKarts(false);

}   // initRest

//=============================================================================
/** Frees all manager and their associated memory.
 */
void PySuperTuxKart::cleanSuperTuxKart()
{
    if(Online::RequestManager::isRunning())
        Online::RequestManager::get()->stopNetworkThread();

    // Stop music (this request will go into the sfx manager queue, so it needs
    // to be done before stopping the thread).
    irr_driver->updateConfigIfRelevant();
    if(race_manager)            delete race_manager;
	race_manager = nullptr;
    if(grand_prix_manager)      delete grand_prix_manager;
	grand_prix_manager = nullptr;
    if(highscore_manager)       delete highscore_manager;
	highscore_manager = nullptr;
    if(attachment_manager)      delete attachment_manager;
	attachment_manager = nullptr;
    ItemManager::removeTextures();
    if(powerup_manager)         delete powerup_manager;
	powerup_manager = nullptr;
    if(projectile_manager)      delete projectile_manager;
	projectile_manager = nullptr;
    if(kart_properties_manager) delete kart_properties_manager;
	kart_properties_manager = nullptr;
    if(track_manager)           delete track_manager;
	track_manager = nullptr;
    if(material_manager)        delete material_manager;
	material_manager = nullptr;
    if(history)                 delete history;
	history = nullptr;
	
    ReplayPlay::destroy();
    ReplayRecorder::destroy();
    ParticleKindManager::destroy();
    PlayerManager::destroy();
    Online::ProfileManager::destroy();
    GUIEngine::DialogQueue::deallocate();
    GUIEngine::clear();
    GUIEngine::cleanUp();
    GUIEngine::clearScreenCache();
    if(font_manager)            delete font_manager;
	font_manager = nullptr;
    
    StkTime::destroy();

    // Now finish shutting down objects which a separate thread. The
    // RequestManager has been signaled to shut down as early as possible,
    // the NewsManager thread should have finished quite early on anyway.
    // But still give them some additional time to finish. It avoids a
    // race condition where a thread might access the file manager after it
    // was deleted (in cleanUserConfig below), but before STK finishes and
    // the OS takes all threads down.

#ifndef SERVER_ONLY
    if (!ProfileWorld::isNoGraphics())
    {
        if (UserConfigParams::m_internet_status == Online::RequestManager::
            IPERM_ALLOWED && !NewsManager::get()->waitForReadyToDeleted(2.0f))
        {
            Log::info("Thread", "News manager not stopping, exiting anyway.");
        }
        NewsManager::deallocate();
    }
#endif

    if (Online::RequestManager::get()->waitForReadyToDeleted(5.0f))
    {
        Online::RequestManager::deallocate();
    }
    else
    {
        Log::warn("Thread", "Request Manager not aborting in time, proceeding without cleanup.");
    }

    // The add-ons manager might still be called from a currenty running request
    // in the request manager, so it can not be deleted earlier.
#ifndef SERVER_ONLY
    if(addons_manager)  delete addons_manager;
#endif

    ServersManager::deallocate();
    cleanUserConfig();

    StateManager::deallocate();
    GUIEngine::EventHandler::deallocate();
}   // cleanSuperTuxKart

//=============================================================================
/**
 * Frees all the memory of initUserConfig()
 */
void PySuperTuxKart::cleanUserConfig()
{
    if(stk_config)              delete stk_config;
	stk_config = nullptr;
    if(translations)            delete translations;
	translations = nullptr;
    if (user_config)
    {
        // In case that abort is triggered before user_config exists
        if (UserConfigParams::m_crashed) UserConfigParams::m_crashed = false;
        user_config->saveConfig();
        delete user_config;
		user_config = nullptr;
    }

    if(irr_driver)              delete irr_driver;
	irr_driver = nullptr;
}   // cleanUserConfig
