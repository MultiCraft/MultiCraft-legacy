import Foundation
import ZipArchive
import Sentry

private enum Constants {
	static let percentProgressIndex: Int = 0
}

private struct Asset {
	let name: String
	let path: UnzipPath

	var destinationPath: String {
		switch path {
		case .documents:
			return NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]

		case .library:
			let path = NSSearchPathForDirectoriesInDomains(.libraryDirectory, .userDomainMask, true)[0]
			return (path as NSString).appendingPathComponent("Application Support")
		}
	}
}

private enum UnzipPath: Int {
	case documents
	case library
}

final class VersionManager {
	class func readVersion(withPath path: String?) -> UInt32 {
		let filename = URL(fileURLWithPath: path ?? "").appendingPathComponent("_version").path
		var content: String? = nil
		do {
			content = try String(contentsOfFile: filename, encoding: .ascii)
		} catch {
			return 0
		}
		return UInt32(Int(content ?? "") ?? 0)
	}

	class func parseVersion() -> UInt32 {
		let revstr = Bundle.main.infoDictionary?["CFBundleVersion"] as? String
		let revision = UInt8(Int(revstr ?? "") ?? 0)
		let ret = UInt32(revision | (2 << 24))

		print("App revision \(revstr ?? "") -> \(ret)")
		return ret
	}

	class func writeVersion(withPath path: String?, ver: UInt32) {
		let filename = URL(fileURLWithPath: path ?? "").appendingPathComponent("_version").path
		let content = "\(ver)"
		do {
			try content.write(toFile: filename, atomically: false, encoding: .ascii)
		} catch {
			SentrySDK.capture(error: error)
		}
	}
}

final class ZipManager: NSObject {
	private var assets: [Asset] = [.init(name: "assets", path: .library)]

	@objc func runProcess(_ progress: @escaping (_ percent: Int) -> Void, _ errorBlock: @escaping (Error) -> Void) {
		let versionRuntime = VersionManager.parseVersion()

		for (index, asset) in assets.enumerated() {
			let zippath = Bundle.main.path(forResource: asset.name, ofType: "zip") ?? ""
			let versionDisk = VersionManager.readVersion(withPath: asset.destinationPath)

			#if !DEBUG
			if versionDisk == versionRuntime {
				continue
			}
			#endif

			unzipFile(at: zippath, to: asset.destinationPath, vRuntime: versionRuntime, { (percent) in
				if index == Constants.percentProgressIndex {
					progress(percent)
				}
			}, errorBlock)
		}
	}
}

private extension ZipManager {
	func unzipFile(at path: String, to destination: String, vRuntime: UInt32, _ block: @escaping (_ percent: Int) -> Void, _ errorBlock: @escaping (Error) -> Void) {
		let fileManager = FileManager.default
		let files = (try? fileManager.contentsOfDirectory(atPath: destination)) ?? []

		for file in files {
			do {
				try fileManager.removeItem(atPath: (destination as NSString).appendingPathComponent(file))
			} catch {
				print(error)
			}
		}

		SSZipArchive.unzipFile(atPath: path, toDestination: destination, overwrite: true, password: ZIPPWD, progressHandler: { (file, zipInfo, progress, total) in
			block(progress * 100 / total)
		}) { (path, success, error) in
			if let error = error {
				SentrySDK.capture(error: error)
				errorBlock(error)
			} else {
				block(100)
			}
		}

		VersionManager.writeVersion(withPath: destination, ver: vRuntime)
	}
}
