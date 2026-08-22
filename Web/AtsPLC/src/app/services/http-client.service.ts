import { Injectable } from '@angular/core';
import { Observable, from } from 'rxjs';
import { DataPaging, DisplayData, DeviceConfig, ForceRefreshTarget, KeyPress, WorkMode } from '../main/models';

const deviceConfigPath: string = '/devconfig';
const bitmapPath: string = '/bitmap';
const keyPressPath: string = '/keypress';
const workModePath: string = '/workmode';
const programUploadPath: string = '/program/upload';
const programDownloadPath: string = '/program/download';

interface QueuedRequest {
	taskFactory: () => Promise<any>;
	resolve: (value: any) => void;
	reject: (reason?: any) => void;
}

@Injectable({
	providedIn: 'root'
})
export class HttpClientService {

	private lastETag: string | null = null;

	private concurrencyLimit: number = 8;
	private activeRequests: number = 0;
	private requestQueue: QueuedRequest[] = [];

	private enqueue<T>(taskFactory: () => Promise<T>): Promise<T> {
		return new Promise<T>((resolve, reject) => {
			this.requestQueue.push({ taskFactory, resolve, reject });
			this.processQueue();
		});
	}

	private processQueue(): void {
		while (this.activeRequests < this.concurrencyLimit && this.requestQueue.length > 0) {
			const request = this.requestQueue.shift();
			if (!request) {
				return;
			}

			this.activeRequests++;

			request.taskFactory()
				.then(result => {
					request.resolve(result);
				})
				.catch(error => {
					request.reject(error);
				})
				.finally(() => {
					this.activeRequests--;
					this.processQueue();
				});
		}
	}

	constructor() { }

	private async fetchWithTimeout(url: string, options: RequestInit = {}, timeoutMs: number = 10000): Promise<Response> {
		const controller = new AbortController();
		const id = setTimeout(() => controller.abort(), timeoutMs);

		try {
			const response = await fetch(url, {
				...options,
				signal: controller.signal
			});
			clearTimeout(id);
			return response;
		} catch (error) {
			clearTimeout(id);
			throw error;
		}
	}


	private async throwOnError(response: Response): Promise<void> {
		if (response.ok) {
			return;
		}

		let message = `HTTP error! status: ${response.status}`;
		try {
			const data = await response.json();
			if (data && data.error) {
				message = data.error;
			}
		} catch {
		}
		throw new Error(message);
	}

	public setConcurrencyLimit(limit: number): void {
		this.concurrencyLimit = limit;
	}

	public getDeviceConfig(): Observable<DeviceConfig> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(deviceConfigPath)
				.then(async (response) => {
					await this.throwOnError(response);
					return response.json() as Promise<DeviceConfig>;
				})
		);
		return from(promise);
	}

	public getBitmap(): Observable<DisplayData> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(bitmapPath, {}, 30000)
				.then(async (response) => {
					await this.throwOnError(response);

					const dataPagingHdr = response.headers.get('X-DataPaging');
					if (!dataPagingHdr) {
						throw new Error('no header "X-DataPaging"');
					}
					const dataPaging = JSON.parse(dataPagingHdr) as DataPaging;

					const forceRefreshHdr = response.headers.get('X-ForceRefresh');
					const parsedForceRefresh = forceRefreshHdr ? Number(forceRefreshHdr) : NaN;
					const forceRefresh = Number.isInteger(parsedForceRefresh)
						? parsedForceRefresh
						: ForceRefreshTarget.None;

					const newETag = response.headers.get('ETag');
					if (newETag && newETag === this.lastETag) {
						return {
							dataPaging: dataPaging,
							bitmap: new ArrayBuffer(0),
							forceRefresh: forceRefresh
						} as DisplayData;
					}
					this.lastETag = newETag;

					const arrayBuffer = await response.arrayBuffer();
					return {
						dataPaging: dataPaging,
						bitmap: arrayBuffer,
						forceRefresh: forceRefresh
					} as DisplayData;
				})
		);
		return from(promise);
	}

	public postKeyPress(keyPress: KeyPress): Observable<any> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(keyPressPath, {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(keyPress)
			}).then(async (response) => {
				await this.throwOnError(response);
				return null;
			})
		);
		return from(promise);
	}

	public getWorkMode(): Observable<WorkMode> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(workModePath)
				.then(async (response) => {
					await this.throwOnError(response);
					const data = await response.json();
					return data.mode as WorkMode;
				})
		);
		return from(promise);
	}

	public setWorkMode(mode: WorkMode): Observable<void> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(workModePath, {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ mode: mode })
			}).then(async (response) => {
				await this.throwOnError(response);
			})
		);
		return from(promise);
	}

	public uploadProgram(file: File): Observable<void> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(programUploadPath, {
				method: 'POST',
				body: file
			}, 60000).then(async (response) => {
				await this.throwOnError(response);
			})
		);
		return from(promise);
	}

	public downloadProgram(): Observable<Blob> {
		const promise = this.enqueue(() =>
			this.fetchWithTimeout(programDownloadPath, {}, 60000)
				.then(async (response) => {
					await this.throwOnError(response);
					return response.blob();
				})
		);
		return from(promise);
	}
}