module app;

import hw.gba;
import hw.memory;
import hw.keyinput;

import util;

import diag.log;

import std.conv;
import std.file;
import std.uri;
import std.algorithm.searching: canFind;
import std.mmfile;
import std.file;
import std.path;
import save;

import core.sync.mutex;

import bindbc.sdl;
import bindbc.opengl;

import tools.profiler.profiler;
import ui;

import commandr;

version (gperf) {
	import gperftools_d.profiler;
}

void main(string[] args) {
	// dfmt off
	auto a = new Program("gamebean-emu", "0.1").summary("GameBean Advance")
		.add(new Flag("v", "verbose", "turns on more verbose output").repeating)
		.add(new Option("s", "scale", "render scale").optional.defaultValue("1"))
		.add(new Argument("rompath", "path to rom file"))
		.add(new Option("b", "bios", "path to bios file").optional.defaultValue("./gba_bios.bin"))
		.add(new Flag("k", "bootscreen", "skips bios bootscreen and starts the rom directly"))
		.add(new Option("m", "mod", "enable mod/extension"))
		.add(new Option("p", "profile", "profile the emu (pass in an ELF file here)"))
		.add(new Option("t", "cputrace", "display cpu trace on crash").optional.defaultValue("0"))
		.parse(args);
	// dfmt on

	util.verbosity_level = a.occurencesOf("verbose");

	auto mem = new Memory();

	bool is_beancomputer = a.option("mod").canFind("beancomputer");
	if (is_beancomputer) log!(LogSource.INIT)("BeanComputer enabled");

	KeyInput key_input = new KeyInput(mem);
	auto bios_data = load_rom_as_bytes(a.option("bios"));
	GBA gba = new GBA(mem, key_input, bios_data, is_beancomputer);

	// load rom
	auto rom_path = a.arg("rompath");
	log!(LogSource.INIT)("Loading rom from: %s.", rom_path);

	// check file
	if (uriLength(rom_path) > 0) {
		import std.net.curl : download;
		import std.path: buildPath;
		import std.uuid: randomUUID;

		auto dl_path = buildPath(tempDir(), randomUUID().toString());
		download(rom_path, dl_path);

		auto rom_data = load_rom_as_bytes(dl_path);

		log!(LogSource.INIT)("Downloaded %s bytes as %s", rom_data.length, dl_path);

		gba.load_rom(rom_data);
	} else if (std.file.exists(rom_path)) {
		gba.load_rom(rom_path);
	} else {
		error("rom file does not exist!");
	}

	if (a.flag("bootscreen")) gba.skip_bios_bootscreen();

	auto profile = a.option("profile");
	if (profile) {
		g_profile_gba = true;
		g_profiler    = new Profiler(gba, profile);
	}
	MultiMediaDevice frontend = new RengMultimediaDevice(gba, to!int(a.option("scale")));
	gba.set_frontend(frontend);
	g_log_gba = gba;

	gba.set_internal_sample_rate(16_780_000 / 48000);
	Runner runner = new Runner(gba, frontend);

	Savetype savetype = detect_savetype(gba.memory.rom.get_bytes());
	
	if (savetype != Savetype.NONE && savetype != Savetype.UNKNOWN) {
		Backup save = create_savetype(savetype);
		gba.memory.add_backup(save);

		auto save_path = rom_path.stripExtension().setExtension(".bsv");
		if (!save_path.exists()) {
			ubyte[] save_data = new ubyte[save.get_backup_size()];
			save_data[0..save.get_backup_size()] = 0xFF;
			write(save_path, save_data);
		}

		MmFile mm_file = new MmFile(save_path, MmFile.Mode.readWrite, save.get_backup_size(), null, 0);

		save.deserialize(cast(ubyte[]) mm_file[]);
		save.set_backup_file(mm_file);
	}

	runner.run();

	version (gperf) {
		log!(LogSource.DEBUG)("Started profiler");
		ProfilerStart();
	}

	version (gperf) {
		ProfilerStop();
		log!(LogSource.DEBUG)("Ended profiler");
	}
}
