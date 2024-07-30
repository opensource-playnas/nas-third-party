gclient_gn_args_from = 'src'
vars = {
'chromium_version':
    '103.0.5060.134',
	'chromium_git': 'https://chromium.googlesource.com'
}

deps = {
  'src': (Var("chromium_git")) + '/chromium/src.git@' + (Var("chromium_version")),
}

recursedeps = [
  'src',
]
