import sys
import ConfigParser

config = ConfigParser.RawConfigParser()
config.read( 'project.conf' )

env = Environment()
env.VariantDir( 'build', 'src', duplicate = 0 )

conf = Configure( env )

env.Append( CPPPATH = [
				'src',
				config.get( 'boost', 'incdir_boost' )
			],
			LIBPATH = [ config.get( 'boost', 'libdir_boost' ) ],
			variant_dir='build'
		)

if sys.platform == 'linux2':
	env.Append( CPPFLAGS = "-fPIC" )


if not conf.CheckLibWithHeader('boost_system', 'boost/system/config.hpp', 'c++' ):
	print 'Did not find library boost_system.'
	Exit(1)

if not conf.CheckLibWithHeader('boost_filesystem', 'boost/filesystem/config.hpp', 'c++' ):
	print 'Did not find library boost_filesystem.'
	Exit(1)

if not conf.CheckLibWithHeader('boost_chrono', 'boost/chrono/config.hpp', 'c++' ):
	print 'Did not find library boost_chrono.'
	Exit(1)

env = conf.Finish()



sequenceStatic = env.StaticLibrary(
	'sequence',
	[
		'src/sequence/BrowseItem.cpp',
		'src/sequence/DisplayUtils.cpp',
		'src/sequence/Sequence.cpp',
	]
)

sequenceShared = env.SharedLibrary(
	'sequence',
	[
		'src/sequence/BrowseItem.cpp',
		'src/sequence/DisplayUtils.cpp',
		'src/sequence/Sequence.cpp',
	]
)

sequenceParserStatic = env.StaticLibrary(
	'sequenceparser',
	[
		'src/sequence/parser/Browser.cpp',
	],
	LIBS = sequenceStatic,
)

sequenceParserShared = env.SharedLibrary(
	'sequenceparser',
	[
		'src/sequence/parser/Browser.cpp',
	],
	LIBS = sequenceStatic,
)

env.Program(
	'lss',
	[
		'app/lss.cpp'
	],
	LIBS = [
		sequenceParserStatic,
		sequenceStatic,
		"boost_chrono",
		"boost_system",
		"boost_filesystem",
		]
)

env.Program(
	'lss_perf_test',
	[
		'app/performance_test.cpp'
	],
	LIBS = [
		sequenceParserStatic,
		sequenceStatic,
		"boost_chrono",
		"boost_system",
		"boost_filesystem",
		]
)
