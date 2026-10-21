# Provenance of carried-over and governing files (rule R19)

Every file below was copied **byte for byte** from the original repository `IWatchSheep/ai-hologram` at commit **`1b8c143`** (tag `archive-2026-10-20`), or (last section) created for the migration and approved on October 20, 2026. SHA-256 values are of the exact committed bytes, with LF line endings. `.gitattributes` keeps the working copy in LF, so `run_checks.ps1` checks each file with PowerShell's built-in `Get-FileHash -Algorithm SHA256`, after first checking that the file has no carriage return. The machine-readable list is `docs/PROVENANCE.sha256`.

To verify a file against the original repository yourself: `git show 1b8c143:<source path> | sha256sum` (in Git Bash) in a clone of `IWatchSheep/ai-hologram`.

## Copied from the original repository

| File here | Source path at `1b8c143` | SHA-256 |
|---|---|---|
| `LICENSE` | `LICENSE` | `6b0382b16279f26ff69014300541967a356a666eb0b91b422f6862f6b7dad17e` |
| `.gitignore` | `.gitignore` | `86bde352d814d4375f2e6e1232ba5ddcb2ce005e4bdeca11930802e5fbff2b64` |
| `docs/AI_Hologram_Project_Brief_v1.1.txt` | `docs/AI_Hologram_Project_Brief_v1.1.txt` | `c55ddf2756c83fc1abef0814b69063a39ebdd93783ca984757046519acf49d5c` |
| `docs/EXPERIMENT_1_PROTOCOL.md` | `docs/EXPERIMENT_1_PROTOCOL.md` | `a37061950c5fb954bfdb6a42ccf83e505db19a46eb7bb6216c6ad326db009341` |
| `docs/PHASE_C_design_inputs.md` | `docs/PHASE_C_design_inputs.md` | `b1c8d91b4e178edcc8bc03dcbb7e1f146a95305b08e8a0a7703b4d6332b9d9d8` |
| `docs/protocol_validation/README.md` | `docs/protocol_validation/README.md` | `1e61176d9b5940f5cca2946a1a23da7829a81980ba1c3b6c6ee4bd84d5ee9f80` |
| `docs/protocol_validation/pilot.py` | `docs/protocol_validation/pilot.py` | `231d29a29b83066838eb723ff365ba63ae5702b3be2a64b1bb04a0a9bcc23d6c` |
| `docs/protocol_validation/ref_c1.py` | `docs/protocol_validation/ref_c1.py` | `751d51381c093d498e36d3bb5290f24742c10c6954a853a1db06d7779ee5ea62` |
| `docs/protocol_validation/ref_two.py` | `docs/protocol_validation/ref_two.py` | `77b423aa2329f38f4fe2a8c6831dc6ca8fc21c6d13829dd1ddbb785ff293bcba` |
| `docs/protocol_validation/t_a1.py` | `docs/protocol_validation/t_a1.py` | `5219f037e7dcdfa49160112fc02bd71c4aa843dba45099f82f5baf9813f73416` |
| `docs/protocol_validation/t_a1b.py` | `docs/protocol_validation/t_a1b.py` | `f339b2edd699e83f168d1d787e0bd78dcb02030c5aaa089508fd28f00b233af4` |
| `docs/protocol_validation/t_a2.py` | `docs/protocol_validation/t_a2.py` | `4297d9f08a53148c73215e9d8bd82e0ebf9dcdba05d524083089fcbcb3c94bd8` |
| `docs/protocol_validation/t_a2b.py` | `docs/protocol_validation/t_a2b.py` | `de3678a5649c77c3e0f46c381cd0c0ab4168666b48f5677b06b3c0ad8a2fe5f7` |
| `docs/protocol_validation/t_cases.py` | `docs/protocol_validation/t_cases.py` | `60d3fa28482d693d5da7c7d40c81d02d999c1d6ec26c1c468376e36c886cece5` |
| `docs/protocol_validation/t_cases2.py` | `docs/protocol_validation/t_cases2.py` | `ac5cf856d59c506047ffe07def6b98ae025f2a4c8fa20073f652ba6cc1265a03` |
| `docs/protocol_validation/t_cases3.py` | `docs/protocol_validation/t_cases3.py` | `3d4232777f3cdb1b22f3f2681b976b56e62dc0f16932f267b4f3bbb654d343a5` |
| `docs/protocol_validation/t_cases4.py` | `docs/protocol_validation/t_cases4.py` | `6968178d1b0fdff90ca80227d3f12031def51eaa0f9ee9e6bcd89fa499b1c155` |
| `docs/protocol_validation/t_faultfree.py` | `docs/protocol_validation/t_faultfree.py` | `ea3007b3dee7b60a5331a0f591fefa27355fd1418bfca2db4ffcca4a0943f5b4` |
| `docs/protocol_validation/t_faultfree2.py` | `docs/protocol_validation/t_faultfree2.py` | `8e14f741b3abd85e1eee91447a2743b254a84757ec077f64f83993ffc8c01a84` |
| `docs/protocol_validation/t_g9.py` | `docs/protocol_validation/t_g9.py` | `bb53cb91992440a85b7b7fac9ddfbef8690e84e9af3f0f747a2d3cc81484dd70` |
| `docs/protocol_validation/t_loop.py` | `docs/protocol_validation/t_loop.py` | `32582afbb3a7df74cc65eb2af4f51e3b22988915524f69f6ea9314ec3920a9d0` |
| `docs/protocol_validation/t_mmax.py` | `docs/protocol_validation/t_mmax.py` | `17f7dca0a61fca6f4a7f598c215273c8ced6df74eb6e789320cd84612d7af9b4` |
| `docs/protocol_validation/t_trap3.py` | `docs/protocol_validation/t_trap3.py` | `1ff4d61294a61a987ea2fc68762c823642a7d5f37ed7bbd2da0d76d7e3b500ab` |
| `docs/protocol_validation/validation_logs.txt` | `docs/protocol_validation/validation_logs.txt` | `25daab331c9c86e06b00406993ab9a3cbe4c077fcc471d05fd60a6f730e962c8` |
| `tools/reference/stage4/README.md` | `docs/stage4_reference/README.md` | `08016937c570f8d3ab465d96c3b1e6359a3ad7e6b6abc216441eba3d641ffafa` |
| `tools/reference/stage4/expected_output_stage4.txt` | `docs/stage4_reference/expected_output_stage4.txt` | `1a334703f53a6ecfefde12d7112d1385ee09a1cc8fd5185ac05ba2038032c8fe` |
| `tools/reference/stage4/gen_ab.py` | `docs/stage4_reference/gen_ab.py` | `f816abdc4d0c8381ae474601d815ef61c9c83e73f62db3ed69867cf659440136` |
| `tools/reference/stage4/gen_ab2.py` | `docs/stage4_reference/gen_ab2.py` | `ad7032441e98c1d0b0fc38bbf0855e1a2cba81b628bdb09bf57d07c514088fee` |
| `tools/reference/stage4/gen_c.py` | `docs/stage4_reference/gen_c.py` | `034792b950f060d85c93c458f68851a6d9c03bb588bfed5a50ea27d1010a75bb` |
| `tools/reference/stage4/gen_c_output_2026-10-20.txt` | `docs/stage4_reference/gen_c_output_2026-10-20.txt` | `f9b20b39b09f29b2c2fed315f8ac4e396f537998f8f0e9028b27497e94956761` |
| `tools/reference/stage4/gen_demo.py` | `docs/stage4_reference/gen_demo.py` | `cf21eb35c69979f612c0f8de31a3113653372687b31e349b6270a46155fc8dec` |
| `tools/reference/stage4/gen_demo2.py` | `docs/stage4_reference/gen_demo2.py` | `fe50800eea32cde090194952f6138e80cc6c9bb64518aa4e92d5c604e97c03bd` |
| `tools/reference/stage4/gen_score1.py` | `docs/stage4_reference/gen_score1.py` | `01dccaf206e22c3f47e889a4f9d29c98507b7ccd29a99ab040f87ca07c56ff3f` |
| `tools/reference/stage4/gen_score2.py` | `docs/stage4_reference/gen_score2.py` | `6a39ca5d04b6494974139366b7124b358d868a0453b4ce3050c419f266dadb19` |
| `tools/reference/stage4/gen_score3.py` | `docs/stage4_reference/gen_score3.py` | `5daba4ea709c7981a87e0075a75e98aaf8ebfd97f568a3dd4c89ab5374387378` |
| `tools/reference/stage4/ref_c1.py` | `docs/stage4_reference/ref_c1.py` | `751d51381c093d498e36d3bb5290f24742c10c6954a853a1db06d7779ee5ea62` |
| `tools/reference/stage4/ref_csv.py` | `docs/stage4_reference/ref_csv.py` | `2e169cdb9afda6627523ea9e0fe977fd3dfeaa2bc57410ae1f953b4e1276f21a` |
| `tools/reference/stage4/ref_score.py` | `docs/stage4_reference/ref_score.py` | `cc941cf3172bd583dd955ea7bb0ebfa7f2d4ec545c91a759bf00a66b1bc9daca` |
| `tools/reference/stage4/ref_stage4.py` | `docs/stage4_reference/ref_stage4.py` | `8dbd75a7f633bfff7f86ff89bdee78c2c718df470db6d853d633840d9ecce9a4` |
| `tools/reference/stage4/reference_logs.txt` | `docs/stage4_reference/reference_logs.txt` | `c3b498b58fa151da411788e2fb0509a0c20df7ca47c16c91c94a08373bf928df` |
| `docs/history/ai-hologram/docs/AI_Hologram_Project_Brief_v1.txt` | `docs/AI_Hologram_Project_Brief_v1.txt` | `dd5fb6831bdb05d4144712b20d97797f4a61414ad461a07f8e0b686b49874e04` |
| `docs/history/ai-hologram/docs/DECISIONS.md` | `docs/DECISIONS.md` | `67504e1bc24092ac9b003da47da08504ae68e36923cb6bf2952246c95eee3e20` |
| `docs/history/ai-hologram/docs/EXPERIMENT_LOG.md` | `docs/EXPERIMENT_LOG.md` | `7622adf1b037e6d374609a736177441d884fa88aeac550f745851365f0b81c37` |
| `docs/history/ai-hologram/docs/chatgpt_reply.txt` | `docs/chatgpt_reply.txt` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `docs/history/ai-hologram/.github/copilot-instructions.md` | `.github/copilot-instructions.md` | `85bbb6f2e7eedfdb8af52a365fb4fd2a701d98030c153c8aa44c762db9558dae` |
| `docs/history/ai-hologram/.cursor/cli.json` | `.cursor/cli.json` | `bd44c139a52c1c0da84372b24fa9e4e168ce07b7cf358eb9215836d65b84c276` |
| `docs/history/ai-hologram/.cursor/rules/ai-hologram.mdc` | `.cursor/rules/ai-hologram.mdc` | `79cba7f17455817833938083328317f6d8e9dccd8950ab05e10d6df0a2a5eb03` |
| `docs/history/ai-hologram/README.md` | `README.md` | `be870cc9701d06d48ff26594eab91425846bdc9367c07ff471a356fa51c3c6cc` |
| `docs/history/ai-hologram/CMakeLists.txt` | `CMakeLists.txt` | `99f2afed2ff88c975f8e2f201a649a54dea1fc173acabee2b156cc576f881ccf` |
| `docs/history/ai-hologram/docs/stages/STAGE_00.md` | `docs/stages/STAGE_00.md` | `c4c348e625ad9d01dca3898bfec95a7b82acfd2e2d1d74c79fe741e89affb266` |
| `docs/history/ai-hologram/docs/stages/STAGE_01.md` | `docs/stages/STAGE_01.md` | `29bc93eddd7fbc0176f1744808f3d17d9e496a1a8ad4afc0fcbdd5d21c8cfdb0` |
| `docs/history/ai-hologram/docs/stages/STAGE_02.md` | `docs/stages/STAGE_02.md` | `a47ba43a19dbc678d3706bba83f806f0bbceddb52896649fb2f55240dc97255a` |
| `docs/history/ai-hologram/docs/stages/STAGE_03.md` | `docs/stages/STAGE_03.md` | `f54b318ec4a7f3a7f1363798cac57b6337799df96a5ae755aef20054f175b842` |
| `docs/history/ai-hologram/docs/stages/STAGE_03_owner_words.txt` | `docs/stages/STAGE_03_owner_words.txt` | `6565947e0b5e67d2b74f13ddd450a9567d279be23ad0c5cd3257209b5b444735` |
| `docs/history/ai-hologram/docs/stages/STAGE_04.md` | `docs/stages/STAGE_04.md` | `f0108ff6c0ddd82170a50726faf18b3acd462b0a4385cd7a20018a05a71a64a7` |
| `docs/history/ai-hologram/docs/copilot_prompts/README.md` | `docs/copilot_prompts/README.md` | `2b244b3ea623387571ee70009de830f3288bf42d5cf84d779d438df9e26f0992` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_2a_prompt.txt` | `docs/copilot_prompts/stage_2a_prompt.txt` | `b947f31ba76ba42066d2b35ad53103ab860c2fdabf8e2f8d5b4ce124e32ac6b5` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_2b_followup_1.txt` | `docs/copilot_prompts/stage_2b_followup_1.txt` | `835b5e450f895e60aa637a687492d2b8a8dc17931a500b3b8f1d85ec0cca6f7d` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_2b_prompt.txt` | `docs/copilot_prompts/stage_2b_prompt.txt` | `602c4e0f6e92f78e2c983082a9ec2e3272d085cad2e396b23b1d7b19860c8a33` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_2c_prompt.txt` | `docs/copilot_prompts/stage_2c_prompt.txt` | `a25263c1a7c609cfa7a3bc9f23696f4f143c2479bc05bcf4041910f8d7b7f79e` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3a_followup_1.txt` | `docs/copilot_prompts/stage_3a_followup_1.txt` | `0da9a663abec8ed94c236b0348f479f1847e0d3c5de7f57c74319b98474c5e38` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3a_prompt.txt` | `docs/copilot_prompts/stage_3a_prompt.txt` | `6df7cf2b45edc95cccc655da286a86fd8010875f9f093413d56bb50717ce0dae` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3b_prompt.txt` | `docs/copilot_prompts/stage_3b_prompt.txt` | `e9357b31363eeaac3b484574c3b7c67e702b98862ac2c4189bf71a91cd97174d` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3c_prompt.txt` | `docs/copilot_prompts/stage_3c_prompt.txt` | `ec6514c87189eb342229fcaef312aa15bf9811cd65a1d54b5dcfde906e2a915d` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3d_followup_1.txt` | `docs/copilot_prompts/stage_3d_followup_1.txt` | `ecb883165a496b3c49b40ede36e1fb1a64ec73af72e6856146403aa5c75c1714` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3d_prompt.txt` | `docs/copilot_prompts/stage_3d_prompt.txt` | `e85f0eee34f9deb7fb21cd7cf8873be9dab9881aef0a149c8fa10d8ab97fbd6f` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3e_followup_1.txt` | `docs/copilot_prompts/stage_3e_followup_1.txt` | `8cd01af8e09f8644ddb32685b94b2df430edf3ba3490b1b960cab6e980531ace` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3e_prompt.txt` | `docs/copilot_prompts/stage_3e_prompt.txt` | `6c096b46266315dc41e9223ddf45d79bc0353d82b6e9440f5f5a59532a73805f` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3f_followup_1.txt` | `docs/copilot_prompts/stage_3f_followup_1.txt` | `6645705dd95615f08412135326e98f0cdae1b73234061e40fe2fcdb209c31b54` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3f_followup_2.txt` | `docs/copilot_prompts/stage_3f_followup_2.txt` | `bdce4e3ec62294f6f34a59fe608f67b08f4e86635797ffce8d7b62e82705b356` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_3f_prompt.txt` | `docs/copilot_prompts/stage_3f_prompt.txt` | `6dc199a1acacb4cb5f0d0cccbf74b55c3fb18bd1e864343a7b834158025b1a73` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4a_followup_1.txt` | `docs/copilot_prompts/stage_4a_followup_1.txt` | `7a5168db45290c511bf0443d18125c90ccfcb80ed1d826deef5083b4e2c5c0ef` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4a_prompt.txt` | `docs/copilot_prompts/stage_4a_prompt.txt` | `25e5c5f3e48ada9e650a947872aeb81facc35ead479147c63a5a280ce20801c0` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4b_prompt.txt` | `docs/copilot_prompts/stage_4b_prompt.txt` | `68c5b10a9be8f6e7dfb9fbb4bcf00cc4fa0f01bd65a82031d08b6df0c48527d4` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4c_prompt.txt` | `docs/copilot_prompts/stage_4c_prompt.txt` | `230d4acb4c5066eba02cec7702b173b17e24d0ded721c8eab751cb81ced8cedd` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4d_prompt.txt` | `docs/copilot_prompts/stage_4d_prompt.txt` | `4c79d1a129383e26a45ec1dbf4ab8fc4e0cfb3b754dd3241e001e0d1a36ef163` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4e_prompt.txt` | `docs/copilot_prompts/stage_4e_prompt.txt` | `9037d00d2b248fe258acfac54870624c24bd218dce03f4f5d455dbcce8c28c87` |
| `docs/history/ai-hologram/docs/copilot_prompts/stage_4f_prompt.txt` | `docs/copilot_prompts/stage_4f_prompt.txt` | `d1b0302ebe4636257e8feda3da055a6f74537ca82ca05ccff1792dc7e1af340a` |
| `docs/history/ai-hologram/results/console/program_console_results.txt` | `results/console/program_console_results.txt` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `docs/history/ai-hologram/results/debugging/debugging_output.txt` | `results/debugging/debugging_output.txt` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `docs/history/ai-hologram/results/notes_from_github_copilot/Response_from_Github_copilot.txt` | `results/notes_from_github_copilot/Response_from_Github_copilot.txt` | `6c57db583cb08b8d028b6524a57d9afdb6ade79bf3091a7f9a43ba49eb470605` |

## Governing documents created for the migration (approved October 20, 2026; never edited)

| File | SHA-256 |
|---|---|
| `docs/RULES.md` | `68a48594d06a5cc8db5b511209a3ac61d09a664db5f89873f4148e343f1175c6` |
| `docs/EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md` | `8a16df816d2729ee13910372099ef978617a57af14f08785a16cbbc02691a7a3` |

The original C1 baseline (C++) is not copied: it is frozen at tag `stage-3` (commit `7587f38`) of the original repository, and the rebuilt C1 must pass the equivalence gate of Migration Addendum M1 before it is tagged `c1-baseline-v2`.
