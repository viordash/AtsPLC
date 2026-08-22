import { Component, OnInit, OnDestroy, signal } from '@angular/core';
import { NgIf } from '@angular/common';
import { Subscription } from 'rxjs';
import { HttpClientService } from '../services/http-client.service';
import { ConfirmDialogService } from '../services/confirm-dialog.service';
import { ErrorDialogService } from '../services/error-dialog.service';
import { ForceRefreshService } from '../services/force-refresh.service';
import { WorkMode, ForceRefreshTarget } from '../main/models';

@Component({
	selector: 'app-sidebar',
	standalone: true,
	imports: [NgIf],
	templateUrl: './sidebar.component.html',
	styleUrls: ['./sidebar.component.scss']
})
export class SidebarComponent implements OnInit, OnDestroy {
	collapsed: boolean = false;
	currentWorkMode = signal<WorkMode>(WorkMode.Stop);
	workModeSwitching = signal<boolean>(false);
	WorkMode = WorkMode;

	private forceRefreshSubscription: Subscription | null = null;

	constructor(
		private httpClientService: HttpClientService,
		private confirmDialogService: ConfirmDialogService,
		private errorDialogService: ErrorDialogService,
		private forceRefreshService: ForceRefreshService
	) { }

	ngOnInit(): void {
		this.forceRefreshSubscription = this.forceRefreshService.requests$.subscribe(targets => {
			if (targets & ForceRefreshTarget.WorkMode) {
				this.loadWorkMode();
			}
		});
		this.loadWorkMode();
	}

	ngOnDestroy(): void {
		this.forceRefreshSubscription?.unsubscribe();
	}

	toggleSidebar(): void {
		this.collapsed = !this.collapsed;
	}

	onDownloadClick(): void {
		console.log('Download program');
		this.httpClientService.downloadProgram().subscribe({
			next: (blob) => {
				const url = window.URL.createObjectURL(blob);
				const link = document.createElement('a');
				link.href = url;
				link.download = 'program.dat';
				link.click();
				window.URL.revokeObjectURL(url);
				console.log('Program download completed');
			},
			error: (error) => {
				console.error('Program download failed:', error);
				this.errorDialogService.show(error.message);
			}
		});
	}

	onFileSelected(event: Event): void {
		const input = event.target as HTMLInputElement;
		if (!input.files || input.files.length === 0) {
			return;
		}
		const file = input.files[0];
		input.value = '';

		this.confirmDialogService.show(`Upload program "${file.name}"?`).subscribe(confirmed => {
			if (!confirmed) {
				return;
			}
			console.log('Upload program:', file.name);
			this.httpClientService.uploadProgram(file).subscribe({
				next: () => {
					console.log('Program upload completed');
				},
				error: (error) => {
					console.error('Program upload failed:', error);
					this.errorDialogService.show(error.message);
				}
			});
		});
	}

	onWorkModeClick(mode: WorkMode): void {
		if (this.workModeSwitching() || mode === this.currentWorkMode()) {
			return;
		}

		if (!this.outputsChanged(this.currentWorkMode(), mode)) {
			this.applyWorkMode(mode);
			return;
		}

		const message = mode === WorkMode.Stop
			? 'Stop the program? Outputs will be reset.'
			: 'Start the program? Outputs will follow the program.';

		this.confirmDialogService.show(message).subscribe(confirmed => {
			if (!confirmed) {
				return;
			}
			this.applyWorkMode(mode);
		});
	}

	private outputsChanged(from: WorkMode, to: WorkMode): boolean {
		return (from === WorkMode.Stop) !== (to === WorkMode.Stop);
	}

	private applyWorkMode(mode: WorkMode): void {
		this.workModeSwitching.set(true);
		this.httpClientService.setWorkMode(mode).subscribe({
			next: () => {
				this.workModeSwitching.set(false);
			},
			error: (error) => {
				console.error('Work mode change failed:', error);
				this.workModeSwitching.set(false);
				this.errorDialogService.show(error.message);
			}
		});
	}

	private loadWorkMode(): void {
		this.httpClientService.getWorkMode().subscribe({
			next: (mode) => {
				this.currentWorkMode.set(mode);
			},
			error: (error) => {
				console.error('Work mode read failed:', error);
			}
		});
	}
}
