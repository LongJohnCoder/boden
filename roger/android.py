from resource_file import ResourceFile
import logging, os, sys, shutil

class Roger:
	def __init__(self):
		self.resource_file = ResourceFile()


	def execute(self, args):
		self.resource_file.parse_input(args)

		#if args.action == 'build':
		self.build(args)
		#if args.action == 'dependencies':
		#	self.dependencies(args)

	def full_path(self, path):
		return os.path.normpath(os.path.join(self.resource_file.root_path, path))

	def copy(self, args, src, dest):
		folder = os.path.dirname(dest)
		
		if args.action == 'build':
			logging.info("Copying %s => %s" % ( src, dest ))
			if not os.path.exists(folder):
				os.makedirs(folder)
			shutil.copyfile(src, dest)
		else:
			sys.stdout.write(src)
			sys.stdout.write("\\;")
			sys.stdout.write(dest)
			sys.stdout.write(";")

	def copy_file(self, args, folder_name, src, bundle_path, suffix):
		src = self.full_path(src)

		dest_path = bundle_path.replace('/', '_')
		dest_name, dest_suffix = os.path.splitext(dest_path)
		dest_path = dest_path.replace('.', '_') + dest_suffix

		dest = os.path.join(args.output_directory, folder_name + suffix, dest_path)
		self.copy(args, src, dest)

	def copy_image(self, args, folder_name, bundle_path, resolutions, resolutionId, suffix):
		if resolutionId in resolutions:
			if len(resolutions[resolutionId]) > 0:		
				self.copy_file(args, folder_name, resolutions[resolutionId], bundle_path, suffix)

	def build(self, args):
		logging.debug("Building ...");

		assetlists = self.resource_file.data["assets"]

		for assets in assetlists:
			
			lang_suffix = ""
			if "language" in assets:
				if len(assets["language"]) > 0:
					lang_suffix = "-%s" % (assets["language"])					 

			images = assets["images"]

			for image in images:
				bundle_path = image['bundle_path']
				resolutions = image['resolutions'];

				self.copy_image(args, "drawable", bundle_path, resolutions, "0.75x", lang_suffix + "-ldpi")
				self.copy_image(args, "drawable", bundle_path, resolutions, "1.0x",  lang_suffix + "-mdpi")
				self.copy_image(args, "drawable", bundle_path, resolutions, "1.5x",  lang_suffix + "-hdpi")
				self.copy_image(args, "drawable", bundle_path, resolutions, "2.0x",  lang_suffix + "-xhdpi")
				self.copy_image(args, "drawable", bundle_path, resolutions, "3.0x",  lang_suffix + "-xxhdpi")
				self.copy_image(args, "drawable", bundle_path, resolutions, "4.0x",  lang_suffix + "-xxxhdpi")

			raws = assets["raw"]
			for raw in raws:
				bundle_path = raw["bundle_path"]
				filename = raw["file"]

				self.copy_file(args, "raw", filename, bundle_path, lang_suffix)



